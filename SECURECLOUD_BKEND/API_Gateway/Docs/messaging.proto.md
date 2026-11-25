## Messaging.proto — Communication gRPC

### Role
Définit l'interface réseau binaire (Protocol Buffers) entre l'**API Gateway (C++)** et le **Microservice de Messagerie**

### Structure du contrat
**Service `MessagingService` :**
  * `string sender` (`1`) : Identifiant de l'expéditeur.
  * `string recipient` (`2`) : Identifiant du destinataire ou du salon.
  * `string content` (`3`) : Corps textuel du message.* **Message sortant (`ChatMessageResponse`) :**
  * `bool success` (`1`) : Indique si le microservice a bien traité et enregistré le message.
  * `string message_id` (`2`) : Identifiant unique attribué au message par le backend.

### Fichiers générés associés
Compilé via `protoc.exe` avec le plugin `grpc_cpp_plugin.exe` pour produire :
* `messaging.pb.h` / `messaging.pb.cc` (sérialisation des données).
* `messaging.grpc.pb.h` / `messaging.grpc.pb.cc` (transport réseau HTTP/2 et stub client).