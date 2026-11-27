# SecureCloud — Modèle du domaine et règles métier

**Version 0.2 — 01/11/2026** — remplace `Domain.md` V0.1 **Projet SECURECLOUD\[SKELETON\]** — document commun aux équipes **Client** et **Serveur**

Document de référence métier, indépendant de toute technologie. Il évolue par itération. Toute divergence entre le code et ce document se corrige ici d'abord, pas l'inverse.

## 0. Objet et mode d'emploi

Ce document répond à une seule question : **de quoi parle SecureCloud, indépendamment de toute technologie ?** Il ne cite ni bibliothèque, ni protocole, ni base de données. Test rapide : une phrase est bien placée ici si on peut la discuter avec quelqu'un qui ne connaît ni Qt, ni gRPC, ni PostgreSQL.

**Conventions**

- Noms canoniques des concepts en **anglais** (ceux du code), définitions en **français**.
- Portée : **(C)** Client seulement, **(S)** Serveur seulement, **(CS)** les deux.
- Statut d'une décision : **\[acté\]** décidé en équipe ; **\[proposé\]** recommandé, à valider ; **\[à décider\]** ouvert, listé au §8.
- Périmètre : **SK** (prototype), **RM** (roadmap V1/V2), **DS** (écarté), comme dans `specifications-securecloud-skeleton.md`.

**Documents liés** : `specifications-securecloud-skeleton.md` (décisions ADR), `UseCases_Client.md` (cas d'utilisation UC-01 à UC-11), `Roadmap MessagingService` (points reportés après UC1).

## 1. Ce qui change depuis la V0.1

| # | Changement | Raison | Statut |
| --- | --- | --- | --- |
| 1 | Le domaine Client ne connaît que le contenu **en clair** (`PlaintextContent`). Le contenu chiffré (`OpaqueContent`) est une enveloppe technique : DTO applicatif côté Client, donnée opaque côté Serveur. | Le chiffrement intervient à la frontière Application/Infrastructure, jamais dans le domaine. | acté |
| 2 | `Message` n'a **pas de destinataire** : il référence un expéditeur et une `Conversation`. Le routage appartient à la Conversation. | Dans un groupe il y a plusieurs destinataires ; un champ unique serait faux. | acté |
| 3 | `Conversation` **ne contient pas** ses messages. L'historique est chargé par pages via un repository. | Volumétrie (RAM < 50 Mo) et agrégats séparés. Annule l'ancienne idée « Conversation propriétaire de sa liste de messages ». | acté |
| 4 | `Attachment` = **métadonnées seulement** (id, nom, taille, type). Le transport du fichier est technique. | Le domaine ne connaît ni système de fichiers, ni sockets. | acté |
| 5 | Les **contacts** ne font pas partie du domaine : c'est une vue personnalisée d'un autre utilisateur (nom d'affichage, tags), gérée côté Application. | `User` reste centré sur l'identité. | acté |
| 6 | Le **self-chat** est accepté : les deux participants d'une conversation directe peuvent être le même utilisateur. | Besoin fonctionnel (notes personnelles). | acté |
| 7 | Le contenu d'un message est borné **en octets** (`MAX_BYTES`), pas en caractères. | Le chiffrement et le réseau travaillent en octets. | acté |
| 8 | Présence : `ONLINE` / `OFFLINE` en SK, liée à la session. `AWAY` existe dans le type mais est **réservé V1**. | ADR MSG-02, arbitrage du 16/09/2026. | acté |
| 9 | Rôles : trois valeurs fixes, permissions portées par une politique pure. | ADR AUTH-03. | acté |
| 10 | Groupes de discussion : le modèle les anticipe, la fonctionnalité reste **RM**. Un envoi vers un groupe est refusé explicitement. | ADR MSG-04. | acté |
| 11 | Sémantique des statuts de livraison, statut d'un message reçu, entité `Session` côté Client. | Non tranchés. | à décider (§8) |

## 2. Principe structurant : deux points de vue sur le contenu

Le contenu d'un message ou d'un fichier n'a pas la même réalité selon l'endroit où l'on se place. **Côté Client** il existe en clair : c'est le seul endroit où le domaine « voit » ce qui est écrit. **Côté Serveur** il n'existe que comme **donnée opaque** : le serveur manipule des enveloppes, pas des messages lisibles.

| Information | Client | Serveur |
| --- | --- | --- |
| Texte du message | clair | opaque, jamais déchiffrable |
| Octets d'une pièce jointe | clairs | opaques |
| Nom de fichier d'une pièce jointe | clair | **à décider** (O-1) |
| Expéditeur, conversation, destinataires | connus | **visibles** (nécessaires au routage) |
| Priorité du message | connue | **visible** (sert au routage) |
| Horodatage, taille de l'enveloppe | connus | **visibles** |
| Statut de livraison | connu | connu |
| Identité (login, rôle) | connue | connue (nécessaire pour authentifier) |

Ce n'est pas un détail de chiffrement, c'est une **frontière de domaine**. Promesse exacte : le serveur ne voit jamais le **contenu** ; elle ne signifie pas que le serveur ne sait rien de l'utilisateur.

## 3. Vocabulaire ubiquitaire

| Terme | Nom canonique | Définition | Portée | Périmètre |
| --- | --- | --- | --- | --- |
| Utilisateur | `User` | Personne disposant d'un compte, identifiée de façon unique. | CS | SK |
| Rôle | `Role` | Catégorie fixe d'un utilisateur : `USER`, `COORDINATOR`, `ADMINISTRATOR`. | CS | SK simplifié |
| Permission | `Permission` | Autorisation d'effectuer une action, portée par un rôle. | CS | SK simplifié |
| Session | `Session` | Période pendant laquelle un utilisateur est reconnu comme authentifié. | CS | SK |
| Présence | `PresenceStatus` | Disponibilité déclarée : `ONLINE`, `OFFLINE` (`AWAY` réservé V1). | CS | SK |
| Conversation directe | `DirectConversation` | Échange entre deux participants (éventuellement le même utilisateur). | CS | SK |
| Groupe de discussion | `GroupConversation` | Échange entre plusieurs utilisateurs. | CS | RM |
| Message | `Message` | Unité de communication, écrite par un utilisateur dans une conversation. | CS | SK |
| Pièce jointe | `Attachment` | Fichier associé à un message (métadonnées dans le domaine). | CS | SK simplifié |
| Groupe d'alerte | `AlertGroup` | Population nommée, cible d'une diffusion d'alerte. Distinct d'un groupe de discussion. | CS | SK |
| Alerte | `Alert` | Communication à diffusion large, suivie individuellement jusqu'à acquittement. | CS | SK |
| Acquittement | `Acknowledgment` | Confirmation individuelle qu'un destinataire a pris connaissance d'une alerte. | CS | SK |
| Événement d'audit | `AuditEvent` | Trace d'une action de sécurité ou d'administration, jamais du contenu. | S | SK |

**Objets-valeur** : `UserId`, `UserLogin`, `MessageId`, `ConversationId`, `AttachmentId`, `TimeStamp`, `Priority` (`NORMAL`, `HIGH`, `CRITICAL`), `MessageContent` (Client).

## 4. Fiches d'entités

### 4.1 User (CS, SK)

Une personne autorisée à utiliser SecureCloud. Attributs conceptuels : identifiant unique et stable, login, rôle, présence, date de création.

- Un utilisateur a **toujours exactement un rôle**.
- Sans session active, aucune action nécessitant une identité n'est possible.
- Le login suit la même règle de normalisation des deux côtés (BR-11).
- Le mot de passe n'est **jamais** un attribut du domaine Client.
- Un utilisateur n'est pas sa session : il peut en avoir eu plusieurs successivement.

### 4.2 Session (CS, SK)

Reconnaissance, par le système, qu'un utilisateur est actuellement authentifié.

- Toujours associée à un utilisateur.
- Peut être **révoquée** ; une session révoquée ne redevient jamais active.
- Côté Client, la session est aujourd'hui un **état applicatif** (jetons, utilisateur courant). Savoir si elle devient aussi une entité du domaine Client est une décision ouverte (O-7).

### 4.3 DirectConversation (CS, SK)

Le cadre dans lequel deux participants échangent des messages.

- Deux participants, **éventuellement identiques** (self-chat).
- **L'identité** d'une conversation est son `ConversationId`, jamais la paire de participants.
- Il existe **au plus une** conversation directe par paire non ordonnée de participants, self-chat inclus (BR-07). Cette règle ne peut pas être portée par l'entité seule : elle est garantie par la recherche préalable (use case) et par une contrainte d'unicité du stockage sur la paire canonique.
- Elle naît au premier message envoyé (BR-08).
- Elle ne contient pas ses messages.

### 4.4 GroupConversation (CS, RM)

Cadre d'échange entre plusieurs utilisateurs. Le **comportement complet** (diffusion à N destinataires, permissions dynamiques, gestion des clés de groupe) est reporté en V1. Le **modèle anticipe** son arrivée : `DirectConversation` et `GroupConversation` partagent une base commune `Conversation`, pour que les groupes soient une extension et jamais une réécriture. Tant que la fonction n'existe pas, un envoi vers un groupe est refusé explicitement (BR-21).

### 4.5 Message (CS, SK)

Attributs conceptuels : identifiant, expéditeur, conversation, contenu, priorité, horodatage de création et d'envoi, statut de livraison, pièces jointes.

- Jamais sans expéditeur ni conversation. **Pas d'attribut destinataire.**
- Le contenu en clair n'est jamais une donnée que le Serveur connaît ou dérive (BR-09).
- La priorité est visible du Serveur ; le contenu ne l'est pas.
- Le statut de livraison progresse dans un sens, avec une boucle de reprise (BR-12).

### 4.6 Attachment (CS, SK simplifié)

Métadonnées : identifiant, nom de fichier, taille, type MIME. Toujours rattachée à un message existant. Deux pièces jointes ne sont jamais reconnues identiques par le serveur (pas de déduplication, ADR FILE-02). Le pipeline technique (chemin, hachage, découpage, chiffrement, envoi) n'appartient pas au domaine.

### 4.7 AlertGroup, Alert, Acknowledgment (CS, SK)

- `AlertGroup` : nom et au moins un membre pour être utilisable. L'appartenance est indépendante de celle d'un groupe de discussion.
- `Alert` : émetteur autorisé, groupe cible, priorité, contenu, état global, et un `Acknowledgment` attendu par destinataire. La liste des destinataires est **figée à la diffusion**.
- `Acknowledgment` : une alerte, un utilisateur ; **définitif** une fois posé. Tant qu'il n'est pas posé, le destinataire est `NOT_ACKNOWLEDGED` (état par défaut).

### 4.8 Role et Permission (CS, SK simplifié)

Rôles : `USER`, `COORDINATOR`, `ADMINISTRATOR`. Permissions identifiées à ce stade (liste ouverte) : `MESSAGE_SEND`, `MESSAGE_READ`, `FILE_UPLOAD`, `FILE_DOWNLOAD`, `ALERT_EMIT`, `USER_REVOKE`, `AUDIT_READ`. Une action n'est jamais accordée « parce que c'est un administrateur » en dur dans le code, mais parce que le rôle porte la permission (BR-05). Questions ouvertes : héritage entre rôles (O-3) et valeurs échangées sur le réseau (O-9).

### 4.9 AuditEvent (S, SK)

Trace d'une action de sécurité ou d'administration : `USER_LOGIN`, `USER_LOGOUT`, `TOKEN_REVOKED`, `FILE_UPLOADED`, `PERMISSION_CHANGED`... Jamais de contenu clair, de mot de passe, de jeton ni de clé privée (BR-18).

## 5. Cycles de vie

### 5.1 Session

`Created` (connexion réussie) → `Active` → `Renewed` (rafraîchissement) → `Active`... puis soit `Expired` (durée atteinte sans rafraîchissement), soit `Revoked` (action explicite, définitif).

### 5.2 Message (côté expéditeur)

| Statut | Sens retenu |
| --- | --- |
| `DRAFTED` | Créé localement, jamais transmis ni mis en file. |
| `BUFFERED` | Validé par l'utilisateur, en file d'envoi, conservé durablement. |
| `SENDING` | Transmission en cours. |
| `SENT` | Voir ci-dessous. |
| `DELIVERED` | Voir ci-dessous. |
| `READ` | Le destinataire a pris connaissance du message. |
| `UNDELIVERED` | Échec ; la cause est soit la connexion, soit le serveur. Reprise possible vers `BUFFERED`. |

**Point non tranché (O-5).** Deux lectures coexistent. *Lecture A* (documentation actuelle du code) : `SENT` = acquitté par le serveur, `DELIVERED` = acquitté par l'appareil du destinataire. *Lecture B* (scénario d'envoi décrit par l'équipe Client) : `SENT` = transmis depuis le client, `DELIVERED` = la passerelle a validé et enregistré le message à destination du destinataire. Il faut une seule définition, partagée par le code, ce document et le contrat Client-Serveur. Voir la Roadmap MessagingService, point R3.

**Message reçu (O-6).** Le statut initial d'un message côté destinataire n'est pas encore défini (point R2).

### 5.3 Alert

`Created` → `Broadcast` (destinataires figés) → `Received` (par chaque destinataire, indépendamment) → `Acknowledged` ou `NotAcknowledged` (défaut) → `Closed` (décision de l'émetteur, optionnelle).

### 5.4 Transfert d'une pièce jointe

`Pending` → `InProgress` (par blocs) → `Complete` → `Available`, ou `Failed` (reprise possible, au mieux).

## 6. Règles métier

| ID | Règle | Portée | Statut | Référence |
| --- | --- | --- | --- | --- |
| BR-01 | Envoyer un message, émettre une alerte ou accéder à une pièce jointe exige une session active. | CS | acté | UC-02 à UC-11 |
| BR-02 | Une session révoquée ne redevient jamais active. « Immédiat » signifie : aucun nouveau traitement authentifié n'est accepté après propagation de la révocation. | CS | acté | AUTH-01 |
| BR-03 | Créer une alerte exige la permission `ALERT_EMIT`. | CS | acté | UC-10 |
| BR-04 | Révoquer la session d'un autre utilisateur exige `USER_REVOKE`. | S | acté | AUTH-01 |
| BR-05 | Une autorisation découle de la permission portée par le rôle, jamais d'un nom de rôle écrit en dur. | CS | acté | AUTH-03 |
| BR-06 | Un message appartient à exactement une conversation et n'a pas de destinataire propre ; le routage relève de la conversation. | CS | acté | — |
| BR-07 | Au plus une conversation directe par paire non ordonnée d'utilisateurs, self-chat inclus. Garantie par la recherche préalable et par l'unicité de la paire canonique au stockage. | CS | acté | — |
| BR-08 | Une conversation directe naît au premier message. Le serveur la crée à la réception de ce premier message si elle est absente, et refuse si l'identifiant existe avec d'autres participants. | CS | proposé | R5 |
| BR-09 | Le serveur ne connaît jamais le contenu. Sont visibles : expéditeur, conversation, destinataires, priorité, horodatage, taille. | CS | acté | SEC-03 |
| BR-10 | Le contenu d'un message est non vide et borné en octets (`MAX_BYTES`, 4096 dans le code actuel). | C | acté | — |
| BR-11 | Le login est normalisé de la même façon des deux côtés : espaces de bord retirés, espaces internes refusés, jeu de caractères limité. La casse reste à décider (O-8). | CS | acté / à décider | AUTH-02 |
| BR-12 | Le statut de livraison avance dans un sens ; seule la reprise `UNDELIVERED` → `BUFFERED` revient en arrière. | C | proposé | R3 |
| BR-13 | Un message est identifié de façon unique ; le serveur ignore un identifiant déjà reçu (idempotence), ce qui rend le renvoi sans risque de doublon. | CS | proposé | R3 |
| BR-14 | Une pièce jointe est rattachée à un message existant ; aucune déduplication côté serveur. | CS | acté | FILE-02 |
| BR-15 | Les destinataires d'une alerte sont figés au moment de la diffusion. | CS | acté | MSG-05 |
| BR-16 | Une alerte exige un groupe d'alerte non vide. | CS | acté | MSG-05 |
| BR-17 | Un utilisateur n'acquitte que les alertes dont il est destinataire ; un acquittement est définitif. | CS | acté | MSG-05 |
| BR-18 | Un événement d'audit ne contient jamais de contenu clair, de mot de passe, de jeton ni de clé privée. | S | acté | AUD-01 |
| BR-19 | Un événement d'audit est généré pour toute émission ou clôture d'alerte, toute révocation de session, tout transfert de pièce jointe réussi. | S | acté | AUD-01 |
| BR-20 | La présence `ONLINE` / `OFFLINE` suit l'état de la session. `AWAY` est réservé V1. | CS | acté | MSG-02 |
| BR-21 | Un envoi vers un groupe de discussion est refusé avec une erreur explicite « fonction non disponible », jamais ignoré en silence. | CS | acté | MSG-04 |

## 7. Frontières : ce qui n'est pas ici

Appartiennent à d'autres documents : protocole de transport, format de sérialisation, algorithmes de chiffrement et gestion des clés (`Crypto.md`), structure des tables, éléments d'interface, organisation des conteneurs, forme exacte des classes.

## 8. Points ouverts

| ID | Question | Impact | Statut |
| --- | --- | --- | --- |
| O-1 | Le nom d'une pièce jointe est-il en clair pour le serveur ou inclus dans le contenu opaque ? Un nom comme « Rapport\_Evacuation\_Kaboul.pdf » peut fuiter. | Confidentialité, format du contrat de fichiers | à décider avant le module fichiers |
| O-2 | Que devient un groupe d'alerte dont tous les membres sont désactivés ? | Invariant de `Alert` | à décider |
| O-3 | Un administrateur hérite-t-il des permissions d'un coordinateur ? | Matrice de permissions | à décider |
| O-4 | La relance après non-acquittement est-elle une règle métier avec délai, ou une simple notification ? | UC-11 | à décider |
| O-5 | Définition exacte de `SENT` et `DELIVERED`, transitions vers `UNDELIVERED`. | Code, contrat, tests | à décider (R3) |
| O-6 | Statut initial d'un message reçu. | Code, tests | à décider (R2) |
| O-7 | La session est-elle une entité du domaine Client, ou seulement un état applicatif ? | Architecture Client | à décider avant UC1 |
| O-8 | Casse du login : insensible (normalisé) ou sensible ? Doit être identique Client et Serveur. | Authentification | à décider avec le Serveur |
| O-9 | Valeurs de rôle échangées sur le réseau (la spécification cite `user`, `coordinateur`, `admin`). | Contrat de login | à décider avec le Serveur |
| O-10 | Qui génère les identifiants de message et de conversation ? Un identifiant de conversation dérivé de la paire triée est-il préférable à un UUID ? | Contrat, hors-ligne, idempotence | proposé : le client génère (R3, R5) |

## 9. Traçabilité

| Entité | ADR | Cas d'utilisation | Statut |
| --- | --- | --- | --- |
| `User`, `Session` | AUTH-01, AUTH-02, AUTH-03 | UC-01, UC-09 | SK |
| `PresenceStatus` | MSG-02 | UC-01, UC-09 | SK (`AWAY` RM) |
| `DirectConversation` | MSG-04 | UC-02, UC-05, UC-06 | SK |
| `GroupConversation` | MSG-04 | UC-03, UC-07 | RM (modèle anticipé) |
| `Message` | SEC-02, SEC-03, MSG-01, MSG-03 | UC-02, UC-05, UC-06 | SK |
| `Attachment` | FILE-01, FILE-02, FILE-03 | UC-04, UC-08 | SK simplifié |
| `AlertGroup`, `Alert`, `Acknowledgment` | MSG-05 | UC-10, UC-11 | SK |
| `Role`, `Permission` | AUTH-03 | UC-10 | SK simplifié |
| `AuditEvent` | AUD-01 | transverse | SK |
