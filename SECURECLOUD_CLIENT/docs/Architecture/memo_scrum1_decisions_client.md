# Mémo d'équipe — Scrum d'architecture n°1 (Client Qt)

**Projet :** SECURECLOUD[SKELETON] — prototype Client
**Date :** 28/09/2026
**Objet :** décisions actées, arguments, points restant à trancher
**Rappel de méthode :** Exigence → Analyse → Décision → Spécification → Implémentation → Test → Preuve. Pas de code avant validation de l'architecture ; on modélise d'abord en PlantUML.

---

## 1. Décisions actées

### Structure et couches

| # | Décision | Argument |
|---|---|---|
| D1 | Le schéma en couches (UI / Application / Infrastructure, avec Domain et Shared transverses) est la référence logique. | Règle de dépendance à sens unique : le Domain ne connaît aucune couche externe, ce qui le rend testable seul. |
| D2 | L'arborescence de référence est `arborescence_Git.txt` (racine `SecureCloud-Client/`). Les anciennes propositions sont mises de côté, réservées au futur mock de test (`tests/spikes`). | Une seule structure de référence évite les divergences entre code et documentation. |
| D3 | On repart de zéro pour le code. | Le premier squelette précédait la réflexion architecturale ; le reprendre ferait hériter de choix non validés. |
| D4 | Le dossier `Shared/` est créé dès maintenant, vide au départ. | Constantes, types et erreurs partagés vont apparaître inévitablement ; autant leur donner une place dès le début. |

### Ports et responsabilités

| # | Décision | Argument |
|---|---|---|
| D5 | Les ports vers l'extérieur (`ITransport`, `ICryptoProvider`, `ILocalStorage`) sont définis par la couche Application ; l'Infrastructure les implémente. | Permet le passage mock → réel sans toucher au reste (messagerie sans E2E réel d'abord, crypto réelle ensuite). |
| D6 | Le port `ISessionGateway` (login, refresh, révocation) est distinct d'`ITransport`. `SessionManager` porte l'état de session ; `LoginUserUseCase` orchestre les deux. | Isole la logique de session (token, révocation, cf. AUTH-01) du transport générique et clarifie le déroulement des UseCases. |
| D7 | `domain/services/` contient uniquement des règles métier pures, sans I/O (ex. « qui peut créer une Alert »). L'orchestration et les appels aux ports restent dans les UseCases. | Évite qu'une même règle soit dupliquée entre Domain et Application. |
| D8 | Un UseCase = une classe (`LoginUserUseCase`, `SendMessageUseCase`…). | Responsabilité unique et traçabilité directe avec `UseCases.md` (UC-01 → `LoginUserUseCase`, etc.). |
| D9 | Persistance locale de l'itération 1 : en mémoire. SQLite réel en itération 2. | Hors besoin réel pour la messagerie 1-to-1 de la démo ; même logique mock → réel que pour Transport et Crypto. |

### Modèle du domaine (User, Message, Conversation)

| # | Décision | Argument |
|---|---|---|
| D10 | `OpaqueContent` sort du domaine Client. Le domaine n'expose que `PlaintextContent` ; `OpaqueContent` devient un DTO applicatif (`MessageDTO`, produit par `MessageMapper` via `ICryptoProvider`). | Le domaine Client ne manipule jamais du contenu chiffré : le chiffrement intervient à la frontière Application/Infrastructure. |
| D11 | `Conversation` est une classe abstraite commune ; `DirectConversation` est implémentée (SK), `GroupConversation` est anticipée dans l'interface mais son `addMessage()` échoue explicitement (RM, ADR MSG-04). | L'arrivée des groupes en V1 sera une extension du modèle, jamais une réécriture (directive `Domain.md` §4.4). |
| D12 | `Conversation` est propriétaire de sa liste de `Message` en mémoire, sans pagination pour SKELETON. La sémantique de `history()` est documentée comme « messages actuellement chargés », pas « historique complet ». | Volumétrie négligeable pour une démo (quelques centaines de Ko pour 200 messages). Le contrat reste compatible avec une pagination via `IMessageRepository` en V1. |
| D13 | `User` a toujours exactement un `Role`, sans changement prévu en SKELETON ; pas de `setPresence()` public — la présence bascule avec le cycle de vie de la `Session`. | Invariants de `Domain.md` §4.1 (AUTH-03, présence ONLINE/OFFLINE liée à la Session). |
| D14 | Sur `Message`, la priorité (`NORMAL`/`HIGH`/`CRITICAL`) est visible du serveur, le contenu ne l'est pas ; le statut de livraison ne progresse que dans un sens (Drafted → Sent → Delivered → Read). | La priorité sert au routage ; le contenu reste couvert par la confidentialité E2E (SEC-03, MSG-03). |

| D15 | Les contacts ne font pas partie du Domaine. Ils sont gérés par la couche Application via une interface dédiée `IContactRepository`. | Un contact n'est pas un concept métier du client de messagerie. C'est une vue personnalisée d'un autre utilisateur (nom d'affichage, tags, groupes). Il s'agit d'une agrégation de données techniques, souvent chargée depuis un fichier local (JSON ou CSV). L'entité `User` doit rester centrée sur l'identité et l'authentification, sans être alourdie par des préoccupations de présentation. 

**Conséquence :** `User` ne contient **aucune** liste de contacts. La couche Application fournit les contacts à l'UI sous forme de DTOs.

---

### D2 — Message sans destinataire : la Conversation possède le routage

**Décision :** L'entité `Message` ne contient **pas** d'attribut `receiverId`. Un message appartient à une `Conversation`, et c'est la Conversation qui connaît ses participants.

**Justification :** Un message ne peut pas exister sans conversation. Il est un objet enfant de l'agrégat `Conversation`. Dans un groupe, il n'y a pas un destinataire mais plusieurs — un champ `receiverId` unique serait incorrect. La Conversation est le conteneur de routage naturel.

**Conséquence :** `Message` contient uniquement `senderId` et `conversationId`. Le routage vers les destinataires est la responsabilité de la Conversation et de la couche Application.

---

### D3 — Séparation Mémoire / Persistance : pagination obligatoire

**Décision :** L'entité `Conversation` n'embarque **pas** la liste de ses messages. L'historique est chargé via `IMessageRepository.findByConversation(id, offset, limit)`. Un cache mémoire technique (`MessageCache`) est géré par la couche Application pour les messages « chauds ».

**Justification :** Un client de messagerie peut gérer des conversations contenant des dizaines de milliers de messages. Charger tout l'historique en RAM ferait exploser la consommation mémoire (contrainte : < 50 Mo). La pagination est obligatoire. Le cache mémoire est une optimisation technique, pas une règle métier — il ne doit donc pas polluer le Domaine.

**Conséquence :** `Conversation` ne contient que ses métadonnées (id, type, titre, participants). `Message` référence sa `Conversation` par identifiant. Le chargement de l'historique est paresseux (lazy loading) et paginé.

---

### D4 — Attachments : métadonnées dans le Domaine, fichiers dans l'Infrastructure

**Décision :** L'entité `Attachment` contient uniquement des **métadonnées** (id, nom de fichier, taille, type MIME). La gestion du fichier réel (chemin, hash, chunking, chiffrement, envoi en streaming) est une préoccupation technique traitée par la couche Infrastructure.

**Justification :** Le Domaine ne doit pas connaître les détails du système de fichiers, des sockets ou des bibliothèques cryptographiques. Il exprime un besoin métier (« ce message possède une pièce jointe »), pas la manière de la transporter.

**Conséquence :** Le pipeline technique (chemin → hash → chunk → chiffrement → envoi) est implémenté dans l'Infrastructure derrière un port (`IFileStorage`). Le Domaine ne voit que des `AttachmentId`.

---

### 🚦 Prêt pour le code

Ces décisions sont maintenant consignées. Elles pourront être reprises telles quelles dans votre présentation orale ou votre dossier de spécifications.

Envoyez la commande :

**`//Code value objects//`**

Et nous écrirons, dans l'ordre : `UserId.h`, `Login.h`, `MessageId.h`, `ConversationId.h`, `AttachmentId.h`, `MessageContent.h`.

À vous de jouer.


---

## 2. Points en attente de décision

| # | Sujet | Enjeu / piste |
|---|---|---|
| P1 | Couche `Platform` (System, Container, Hardware) | Absente de l'arborescence pour l'instant. À reconsidérer quand un besoin réel apparaît (réseau dégradé 256 kbps, abstraction OS, stockage local bas niveau). |
| P2 | Orchestration côté UI | Qui appelle plusieurs UseCases et met à jour `MessageListModel` : les fenêtres directement, ou un Presenter/Controller léger ? À trancher avant d'attaquer `ui/`. |
| P3 | Stockage du token | Où persiste-t-il (`ILocalStorage`, `JwtManager`, composant dédié) ? Non bloquant pour le Domain. |
| P4 | `Role` : enum ou entité | Le draft le modélise en enum (3 rôles fixes en SKELETON) avec les vérifications de permission dans un service de domaine. À confirmer à la relecture du diagramme. |
| P5 | Entités restant à modéliser | `Session`, `Attachment`, `AlertGroup`, `Alert`, `Acknowledgment`, puis la matrice des permissions. |
| P6 | Points ouverts de `Domain.md` §8 | Métadonnées de l'`Attachment` dans le contenu opaque ; devenir d'un `AlertGroup` sans membre actif ; héritage des permissions COORDINATOR → ADMINISTRATOR ; règle de relance après non-acquittement (UC-11). |
| P7 | Structure à plat des dossiers | Fixer un seuil (ex. une dizaine de fichiers par dossier) au-delà duquel on crée des sous-dossiers (`ports/`, `usecases/`, `dto/`). |
| P8 | Exception de `GroupConversation::addMessage()` | Nom et forme de l'exception « non supporté » à harmoniser avec le futur module `Errors` de `Shared/`. |
| P9 | Validation du draft PlantUML | Relire le diagramme User / Message / Conversation, puis rédiger les `.h` correspondants (code et commentaires en anglais). |

**Note :** coquille à corriger à la création des fichiers — `Attachement` (avec « e ») dans `ui/`.
