# **SECURECLOUD_CLIENT** `[SKELETON]`
---

<BR>

Architecture : Chargement des Messages (Fonctionnalité 1-to-1)
---

 > **Statut :** Validé pour le SKELETON  
**Portée :** Conversations Directes (1-to-1). Les conversations de groupe (N-to-N) et le chargement de leurs messages complexes sont reportés à la Roadmap V1.  
**Date :** 21/10/2026    

## 1. Objectif et Contrainte Mémoire

Dans le cadre des contraintes du projet MSF (client Qt léger, < 50 Mo de RAM, matériel vieillissant), il est impossible de charger l'intégralité d'une conversation (et de ses milliers de messages) en mémoire au démarrage de l'application.

Le système repose donc sur deux principes :
1. **Le chargement paresseux (Lazy Loading) :** On ne charge que les métadonnées des conversations au démarrage.
2. **La pagination et le cache applicatif :** Les messages sont chargés par blocs (ex: les 50 derniers) uniquement lorsqu'une conversation est ouverte, et stockés dans un cache temporaire.

## 2. Composants du Domaine (Domain Layer)

Le Domaine est pur (C++ sans dépendance à Qt ou SQLite). Il définit les règles métier.

### 2.1 `MessageContent` (Value Object)
Encapsule le texte du message.  
Il est immuable et validé à la construction (max 4096 caractères, suppression des espaces de fin, interdiction des caractères nuls `'\0'`). Il ne sait pas à quelle conversation il appartient.

### 2.2 `Message` (Entité)
Représente un message unique. Il contient :
- Un `MessageId`.
- Un `ConversationId` (référence vers la conversation parente).
- Un `UserId` (expéditeur).
- Un `MessageContent`.
- Un statut (`MessageStatus` : BUFFERED, SENT, etc.).
- Un horodatage (`Timestamp`).

**Règle métier :** Un message est un enfant de la conversation. Il ne peut pas exister sans conversation, mais il n'est **pas** stocké dans l'objet `Conversation`.

### 2.3 `Conversation` (Entité Abstraite) 
La classe `Conversation` représente les métadonnées de la discussion. Elle est abstraite.
#### **`DirectConversation`**  
La classe `DirectConversation` (1-to-1) est finale. 
Elle contient uniquement :
- Un `ConversationId`.
- Un `Timestamp` de création.
+ Les deux `UserId` participants.

#### **`GroupConversation`**  
La classe `GroupConversation` est finale, elle instanciera une discussion de groupe (type chat room). Elle contient uniquement :
- Un `ConversationId`.
- Un `Timestamp` de création.
+ Un `Title`.
+ Une liste de participant `UserId` participants.


**Décision d'architecture :** Ces classes `DirectConversation` et `GroupConversation` **ne contiennent pas** de `std::vector<Message>`. Elle ne contiennent que des métadonnées.  
Nous ne discuterons ici, dans le prototype SKELETON que du fonctionnement de la classe `DirectConversation`. Les conversations de groupe seront très probablement reportés à la V1 (Roadmap).

## 3. Interfaces et Couche Application

**La couche Application orchestre le chargement des données.**

### 3.1 `IMessageRepository` (Interface du Domaine)
Définit le contrat d'accès aux données. L'interface garantit explicitement que les messages retournés sont **triés par date de création croissante**.
```cpp
virtual std::vector<Message> findLastMessages(const ConversationId& id, std::size_t limit) = 0;
```

### 3.2 MessageCache (Application Layer)

C'est le "Hot Stack" (pile chaude). Il s'agit d'un dictionnaire en mémoire (`std::unordered_map`) qui associe un ConversationId à un std::vector<Message>.

 - Il agit comme un tampon de mémoire vive géré par l'application.
 - Il évite de requêter la base de données SQLite à chaque fois que l'utilisateur clique sur une conversation.
 

## 3.3 GetMessagesUseCase (Application Layer)

C'est le chef d'orchestre. Il expose une méthode execute(ConversationId) :
 - Il interroge le MessageCache. Si les messages y sont, il les retourne immédiatement.
 - S'ils n'y sont pas, il appelle IMessageRepository::findLastMessages().
 - Il stocke le résultat dans le MessageCache.
 - Il retourne les messages à l'interface utilisateur (UI).

## 4. Infrastructure et Flux d'Exécution

L'implémentation concrète du repository (ex: SQLiteMessageRepository) vit dans la couche Infrastructure. Elle interagit avec la base de données locale.

***Scénario d'utilisation (User Flow)***
---

### **Étape 1 :** Démarrage de l'application

 + L'UI demande la liste des conversations.
 + La couche Application interroge IConversationRepository.
 + Elle récupère une liste de DirectConversation (métadonnées uniquement).
 + L'UI affiche la liste (nom du contact, date du dernier message).
 + À ce stade, aucun message n'est chargé en RAM.

### **Étape 2 :** L'utilisateur clique sur une conversation

 + L'UI envoie le ConversationId au GetMessagesUseCase.
 + Le Use Case vérifie le MessageCache.
 + S'il est vide, il demande les 50 derniers messages à IMessageRepository.

#### **Étape 3 :** Requête à la base de données

 + SQLiteMessageRepository exécute une requête SQL (ex: SELECT * FROM messages WHERE conversation_id = ? ORDER BY created_at ASC LIMIT 50).
 + Il instancie les objets Message (avec leurs MessageContent).
 + Il renvoie le `std::vector<Message>` au Use Case.

### **Étape 4 :** Mise en cache et affichage

 + Le Use Case stocke le vecteur dans le MessageCache.
 + Le Use Case renvoie le vecteur à l'UI.
 + La QAbstractListModel de Qt affiche les messages.

### **Étape 5** (Optionnelle) **:** Pagination

 - Si l'utilisateur fait défiler vers le haut (scroll), l'UI demande les 50 messages précédents, quand il arrive danss la dernière partie chargée.
 - Le Use Case fait une requête avec un OFFSET (ex: OFFSET 50 LIMIT 50) et ajoute les messages au cache et à l'UI.
 
 

