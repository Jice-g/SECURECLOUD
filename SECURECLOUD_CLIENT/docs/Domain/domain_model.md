# **Présentation du Domaine (Domain Layer)**

## 1. Rôle et Philosophie
Le Domaine est le cœur métier de l'application SecureCloud Client. Il est développé en **C++ pur (STL)**. Il ne connaît ni Qt, ni SQLite, ni les protocoles réseau (WebSocket). Il est totalement indépendant de la couche technique (Infrastructure) et peut être testé en isolation totale.

## 2. Composants du Domaine

### 2.1 Enums (Énumérations)
Ils définissent les états et types fondamentaux du système :
- `UserRole` (ADMIN, ORGA, STD_USER)  
- `PresenceStatus` (ONLINE, OFFLINE, AWAY)  
- `Priority` (LOW, STANDARD, HIGH, CRITICAL)  
- `MessageStatus` (BUFFERED, SENDING, SENT, DELIVERED, UNDELIVERED, READ)  
- `ConversationType` (DIRECT, GROUP)  
  
  
### 2.2 Value Objects (Objets-Valeurs)
Ce sont des objets **immuables** et **validés à la construction**. Ils apportent de la **sûreté du typage** (impossible de confondre un `UserId` avec un `MessageId`) et empêchent "l'obsession des primitifs".  
- **Identifiants :** `UserId`, `MessageId`, `ConversationId`, `AttachmentId` (spécialisations `std::hash` incluses, avec template générique et `struct` spécialisée).  
- **Données validées :** `UserLogin` (normalisation, taille, caractères), `MessageContent` (taille max, suppression des espaces de fin), `Timestamp` (horodatage system_clock).  


### 2.3 Entités (Entities)
Elles possèdent une identité unique et un cycle de vie. Elles contiennent la logique métier (invariants).  
- **`User`** : Représente l'utilisateur local (identité, rôle, présence).  
- **`Conversation`** (abstraite) : Définit le contrat de base des discussions.  
  + **`DirectConversation`** : Conversation 1-to-1 stricte (2 participants, pas de titre).  
  + **`GroupConversation`** : Discussion de groupe (titre obligatoire, liste de participants modifiable).  
- **`Message`** : Le cœur du système. Il possède une machine à états stricte (BUFFERED -> SENDING -> SENT -> DELIVERED -> READ ou -> UNDELIVERED).  
- **`Attachment`** : Métadonnées d'une pièce jointe (nom, taille, type MIME). Le fichier physique est géré par l'Infrastructure.  
  
  
## 3. Règles Métier (Invariants) Clés
Le Domaine garantit en permanence les règles suivantes :
1. **Message (Contenu ou Pièce Jointe) :** Un message valide doit avoir soit du texte, soit au moins une pièce jointe (`isValid()`).
2. **Machine à états du Message :** Il est impossible de passer un message de `BUFFERED` à `DELIVERED` directement. Chaque transition est vérifiée et lève une exception en cas d'erreur.
3. **Conversations Directes :** Impossible de créer un groupe avec le type `DIRECT`, ou d'ajouter un 3ème participant à une `DirectConversation`.
4. **Immuabilité des IDs :** Une fois généré par la couche Application, un identifiant ne peut plus être modifié.

## 4. Gestion de la Mémoire
Conformément aux contraintes MSF (RAM < 50 Mo), l'entité `Conversation` ne contient **pas** la liste de ses messages. Les messages sont chargés à la demande (Lazy Loading) via l'interface `IMessageRepository` et mis en cache dans la couche Application.  

  ---

# **Utilisation des objets du domaine**

## 1. Utiliser `'Message'` pour envoyer un message

**Pourquoi*`Message`*n'a-t-il pas de*`recipientId` *?**  

C'est une question d'architecture fondamentale. Dans une discussion de groupe (N-to-N), un message possède plusieurs destinataires. Si l'on ajoutait un `recipientId` dans la classe `Message`, il faudrait soit dupliquer le message pour chaque destinataire (ce qui casse l'unicité du `MessageId`), soit stocker un tableau de destinataires dans le message (ce qui duplique l'information déjà présente dans `Conversation`).

**La décision architecturale est la suivante :** un `Message` est envoyé à une **Conversation**, pas à un utilisateur. C'est la `Conversation` qui connaît la liste de ses participants (`participant1` et `participant2` pour une `DirectConversation`, `participants` pour une `GroupConversation`).

Le routage (qui doit recevoir quoi) est une responsabilité de la couche **Application** et de l'**Infrastructure**, pas du Domaine.  
Le Domaine se contente de garantir l'intégrité du message et son lien vers la conversation.

## 2. Cas d'Utilisation : "J'envoie un message à Alice"

Voici le cycle de vie complet des objets lors de l'envoi d'un message dans une `DirectConversation`.

### **Étape 1 :** Initialisation
- L'utilisateur ouvre l'application. La couche Application charge ses conversations passées via `IConversationRepository`.
- Une `DirectConversation` avec Alice existe déjà en base de données (ou est créée à la volée si c'est le premier message).
- **État :** `DirectConversation` est chargée en mémoire. Aucun `Message` n'est instancié pour l'instant.

### **Étape 2 :** Rédaction et Envoi (UI -> Application)
- L'utilisateur tape "Bonjour Alice" et clique sur "Envoyer".
- L'UI appelle `SendMessageUseCase.execute(convId, senderId, content)`.

### **Étape 3 :** Création du Message (Application -> Domain)
- La couche Application génère un nouveau `MessageId`.
- Elle instancie un objet `Message`.
- **État du Message :** `BUFFERED`.
- La couche Application appelle `message.isValid()` (vérifie qu'il y a du contenu ou une pièce jointe).
- La couche Application sauvegarde le message dans la base locale (SQLite) via `IMessageRepository` avec le statut `BUFFERED`.

### **Étape 4 :** Chiffrement et Envoi (Application -> Infrastructure)
- La couche Application utilise `ICryptoProvider` pour chiffrer le contenu.
- Elle crée un `SendMessageRequestDTO` (sans le texte en clair).
- Elle passe le DTO à `ITransport` (WebSocket) qui l'envoie au serveur.
- **État du Message :** la couche Application appelle `message.markAsSending()`, puis `message.markAsSent()` lorsque le serveur accuse réception.
- La base locale est mise à jour à chaque transition.

### **Étape 5 :** Réception (Côté Alice)
- Le serveur route le message chiffré vers le client d'Alice (via son `ConversationId`).
- L'Infrastructure d'Alice reçoit le message, le déchiffre, et le passe à la couche Application.
- La couche Application crée un objet `Message` (statut `DELIVERED`), et le sauvegarde en local.
- Quand Alice ouvre la conversation, la couche Application charge les messages via `IMessageRepository` et les met dans le `MessageCache`. Le message passe en statut `READ` si Alice le lit.

### **Résumé du Cycle de Vie**
| Objet | Créé à l'étape | Détruit à l'étape | États traversés |
| :--- | :--- | :--- | :--- |
| `DirectConversation` | Étape 1 (ou antérieure) | À la fermeture de l'app (ou déconnexion) | Chargée en mémoire |
| `Message` (expéditeur) | Étape 3 | Après sauvegarde finale (statut stable) | BUFFERED -> SENDING -> SENT |
| `Message` (destinataire) | Étape 5 (à la réception) | Après lecture | DELIVERED -> READ |