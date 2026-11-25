## 2. `MessagingClient.h` — Client gRPC vers le Microservice de Messagerie

### Rôle
Encapsule toute la complexité réseau **gRPC / Protobuf** dans une classe C++ *Header-Only* simple et sécurisée. Il permet à l'API Gateway d'envoyer des messages au microservice de messagerie sans manipuler directement les objets générés par `protoc`.

### Architecture et mécanismes clés
* **Injection de dépendance (`std::shared_ptr<grpc::Channel>`) :**
  Le canal réseau est fourni au constructeur (`MessagingClient(channel)`). Ce découplage permet d'utiliser un canal TCP réel en production (`grpc::CreateChannel`) et un canal mémoire isolé (`InProcessChannel`) dans les tests unitaires GoogleTest.
* **Validation défensive préalable :**
  La méthode `sendMessage(sender, recipient, content)` vérifie immédiatement qu'aucun des trois paramètres n'est vide. Si une donnée manque, l'appel est rejeté localement (`return false`) sans solliciter le réseau.
* **Protection par timeout (`context.set_deadline`) :**
  Chaque appel gRPC est limité à **2000 ms (2 secondes)**. En cas de lenteur ou de panne du microservice cible, le thread WebSocket de l'API Gateway est libéré proprement au lieu de rester bloqué indéfiniment.
* **Gestion des erreurs réseau (`grpc::Status`) :**
  Vérifie `status.ok()` avant de lire la réponse Protobuf (`response.success()`), garantissant qu'une coupure réseau ne provoque jamais d'exception non gérée.

### Signature principale
```cpp
bool sendMessage(const std::string& sender,
                 const std::string& recipient,
                 const std::string& content);