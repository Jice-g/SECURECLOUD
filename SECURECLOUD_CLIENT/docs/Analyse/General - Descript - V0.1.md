

# **SECURECLOUD_CLIENT - ARCHITECTURE**
<br>  

> **MVP :** prototype SKELETON

<span style="color: stale">
    <b>En documents, l'avancée du projet.</b><br>
    <i>Spécifications - Vision - Domaine - Architecture</i>
</span>  

<br>
<br>

      
## <span style="color: darkslateblue">**Architecture en couche**</span>
<br>

```
                 -------------------
                        UI
                 -------------------
                        ↓
           |     -------------------
   DOMAINE |  ←     APPLICATION
           |     -------------------
           |            ↑
           |     -------------------
   DOMAINE |  ←    INFRASTRUCTURE
           |     -------------------
           |            ↓
           |     -------------------
   DOMAINE |  ←     PLATFORM
           |     -------------------


```

  
 < voir Image>  

   <br>  
   


### <u><span style="color: slateblue">UI | Qt-Widget : Présentation des données et interface visuelle fenêtrée</span></u>
La couche de présentation a pour unique responsabilité d'afficher les données à l'utilisateur et de capturer ses interactions via des widgets Qt. Elle ne contient aucune logique métier et ne communique qu'avec la couche Application, exclusivement par l'intermédiaire de DTOs.  
Elle consomme les Use Cases à travers des contrôleurs, et doit rester réactive (< 2 s au démarrage, < 50 Mo de RAM) même en conditions dégradées (connexion satellite, matériel vieillissant).  
Les contraintes UI sont : multithreading *via* signaux/slots Qt pour ne jamais bloquer le thread principal.  

  * Domain (Model, Repository, Services)  

  * Model : Entities et Value Objects  

<br>  


### <u><span style="color: slateblue">Domain :</span></u>
Le Domaine (domain) contient les règles métier de SecureCloud. Toutes les autres couches existent pour servir le Domaine métier qui ne connait pas UI, WebSocket, SQLite... ou le système d'exploitation.  

Toute couche peut utiliser les objets du Domaine. Le Domaine ne dépend d'aucune couche.

 * Le **model** de domaine constitue le cœur métier du client. Il contient les Entités (`User, Message, Conversation, Contact, Attachment, Alert`), identifiées par un identifiant unique, mutables, porteuses de comportements et responsables de la protection de leurs invariants. Il contient également les Value Objects (`UserId, MessageId, MessageContent, Login...`), immuables, validés à la construction, sans identité propre.

   Ce module est pur C++ : aucune dépendance à Qt, au réseau ou à une base de données  (donc transposable et interfaçable). Il est testable à 100 % isolément.

   Cette possibilité de la tester isolément est essentiel pour stabilisé les règles métier diffusées dans l'ensensemble des couches, packages et fonctionnamlité de l'application.

 * **Repository** : Interfaces, ce sont des classes avec des méthodes virtuelles et virtuelles pures `=0` qui seront implémenté par des instance de classe définit dans la couche ayant la responsabilité des opération qu'elle décrivent. Les *repositories* du domaine sont des interfaces abstraites (`IUserRepository, IMessageRepository, IConversationRepository`) qui définissent les contrats d'accès aux données exprimés en langage métier. Ils expriment le *quoi* (le domaine a besoin de sauvegarder un Message) sans le *comment* (SQLite, MongoDB, fichier…). Ils matérialisent le principe d'inversion de dépendance : l'implémentation concrète réside dans la couche Infrastructure.

 * **MessagingService** : Les *services* du domaine. Comportement sans état qui orchestre la logique métier ne trouvant pas naturellement sa place dans une seule entité. Par exemples : la validation transverse d'un message avant envoi, le calcul de priorité selon le rôle de l'émetteur, ou encore la diffusion d'une alerte à un groupe. Il est appelé par la couche *Application* et ne dépend d'aucun framework.

<br>  



### <u><span style="color: slateblue">Application :</span></u>
La couche Application orchestre les **Cas d'Usage** - CU - (`LoginUser, SendMessage, UploadAttachment`...). Elle reçoit les intentions de l'UI, elle invoque le **Domaine** *via* les *Repositories*, et renvoie des DTOs vers la *présentation* → UI.  
  Elle définit également les Ports (interfaces techniques : `ITransport, ICryptoProvider, ILocalStorage`) que l'**Infrastructure** implémentera.  
  Elle détient le `SessionManager` (gestion des tokens, de l'utilisateur courant, des permissions, etc.). Elle ne contient aucune règle métier — uniquement de la coordination.

<br>  


### <u><span style="color: slateblue">Infrastructure :</span></u>
L'Infrastructure fournit les implémentations techniques concrètes des Ports définis par l'Application. Elle regroupe le transport (`WebSocketClient`), la cryptographie (`LibSodiumProvide`), la persistance locale (`SQLiteRepository`) et la sécurité (`JwtManager, TokenStorage`). Elle utilise la couche **Platform** pour accéder à l'environnement et à l'OS. Elle est remplaçable sans impacter le domaine ni l'application (grâce à une inversion de dépendance). Ce qui permet de rendre encore plus modulaire le déploiement si nécessaire.

<br>  


### <u><span style="color: slateblue">Platform :</span></u>
La couche **Platform** fournit *une abstraction des primitives système et matérielles* : sockets, système de fichiers, conteneurs Docker, horloge système. Elle ne connaît aucun concept applicatif (pas de Message, pas de User). Sa dépendance est unidirectionnelle : `Infrastructure → Platform`, jamais l'inverse. Cette isolation garantit la portabilité Windows / Linux / macOS exigée par le cahier des charges.

<br>  


### <u><span style="color: slateblue">Shared :</span></u>
Module transverse contenant les éléments techniques et réutilisables par toutes les couches : constantes globales, types communs (`Result<T>, Error`), logger, utilitaires de bas niveau.  
Il ne doit contenir aucune logique métier et n'a aucune dépendance vers les autres couches, sous peine de créer des dépendances circulaires.

<br>  

---

<br>  



## Arborescence :

```

SecureCloud-Client/
├── CMakeLists.txt              # The only build file we need
├── README.md
├── LICENSE
├── CHANGELOG.md
├── .gitignore
├── .gitattributes
│
├── docs/                       # ADRs, UML, Specs...
│   ├── school_vision/
│   ├── specifications/
│   ├── decisions/
│   ├── domain/
│   ├── architecture/
│   ├── tests/
│   └── technics/                
│
├── assets/                     # Icons, Images, QSS...
│
├── tests/                      # Test files
│   ├── domain_tests/
│   ├── application_tests/
│   ├── integration_tests/
│   └── spikes/                 # experimental code
│
└── src/
    ├── main.cpp                # Entry point (setup)
    │
   ├── domain/
    │   ├── model/                  # ← Entities + Value Objects
    │   │   ├── Enums.h
    │   │   ├── TimeStamp.h
    │   │   ├── UserId.h
    │   │   ├── UserLogin.h
    │   │   ├── UserLogin.cpp
    │   │   ├── MessageContent.h
    │   │   ├── User.h / .cpp
    │   │   ├── Message.h / .cpp
    │   │   ├── Conversation.h / .cpp
    │   │   ├── Attachment.h / .cpp
    │   │   └── Alert.h / .cpp
    │   │
    │   ├── repositories/           # ← Interfaces
    │   │   ├── IUserRepository.h
    │   │   ├── IMessageRepository.h
    │   │   └── IConversationRepository.h
    │   │
    │   └── services/               # ← Behavior
    │       ├── MessagingService.h / .cpp
    │       └── AlertService.h / .cpp
    │
    ├── application/            # Layer : Application
    │   ├── SendMessageUseCase.h/.cpp
    │   ├── LoginUserUseCase.h/.cpp
    │   ├── MessageDTO.h
    │   ├── UserDTO.h
    │   ├── ITransport.h        # Port
    │   ├── ICryptoProvider.h   # Port
    │   ├── ILocalStorage.h     # Port
    │   ├── MessageMapper.h/.cpp
    │   └── SessionManager.h/.cpp
    │
    ├── infrastructure/        # Layer : Infrstructure 
    │   ├── WebSocketClient.h/.cpp
    │   ├── LibSodiumProvider.h/.cpp
    │   ├── SQLiteRepository.h/.cpp
    │   └── JwtManager.h/.cpp
    │
    └── ui/         # Layer : UI            
        ├── ChatWindow.h
        ├── ChatWindow.cpp
        ├── ChatWindow.ui
        ├── LoginWindow.h
        ├── LoginWindow.cpp
        ├── LoginWindow.ui
        ├── ContactBookWindow.h
        ├── ContactBookWindow.cpp
        ├── ContactBookWindow.ui
        ├── Attachment.h
        ├── Attachement.cpp
        ├── Attachement.ui
        ├── SettingsWindow.h
        ├── SettingsWindow.ui
        ├── MessageListModel.h
        └── MessageListModel.cpp

```
*à compléter le moment venu*

<br><br>


# CAHIER DES SPÉCIFICATIONS : PROJET SECURECLOUD
**SECURECLOUD[SKELETON]** : Le prototype 'Ecole'
> Plateforme de Communications Professionnelles Sécurisées pour Médecins Sans Frontières (MSF)
> Projet d'école Bachelor IT "Développeur logiciel / DevOps" - B3

---

## <span style="color: darkslateblue">LÉGENDE DES DÉCISIONS SKELETON</span>

Chaque spécification tirée du cahier des charges est suivie d'une **Décision SKELETON**, classée selon la méthode définie en Vision École :

| Statut | Signification |
| :--- | :--- |
| **[SK]** | Retenu et implémenté dans SECURECLOUD[SKELETON] — démontré à la soutenance |
| **[RM]** | Reporté en Roadmap (V1/V2) — hors périmètre des 6 semaines, documenté comme suite prévue |
| **[DS]** | Discard — non démontrable ou hors capacité d'un projet étudiant de 3 personnes ; conservé comme limite assumée et documentée |

Chaque décision porte une référence **ADR-ID** (registre de décisions), reprise ou complétée à partir du tableau de la Vision École.
Format : *Spécification (texte du cahier des charges) → Décision SKELETON (phrase simple, prototype minimal)*

---

## 1. CONTEXTE ET ENJEUX STRATÉGIQUES

### 1.1 Contexte Général

#### Projet pédagogique
SECURECLOUD est un projet d'école, par équipe de 3 étudiants, réalisé dans un délai fixé par l'école (six semaines effectives). Le cahier des charges décrit une application pour des organisations humanitaires nécessitant des communications sécurisées. Il est conçu pour remplir des objectifs pédagogiques :
- Développer une application sécurisée
- Concevoir et développer une application sécurisée organisée en couches
- Préparer le déploiement d'une application sécurisée

**Démarche DevOps** : intégration continue, dépôts distants, automatisation des tests, contrôle qualité de code, conteneurisation des environnements de tests/production.

#### Le contenu du cahier des charges
Le cahier des charges présente une vision produit maximaliste (« *absolument inviolable* », « *le serveur ne voit JAMAIS le contenu* ») destinée à un cas d'usage MSF (45 000 volontaires, 70 pays, zones de conflit).

> ➜ **Décision SKELETON [DS]** — *(ADR SEC-01)* : Le texte « inviolable » / « impossibilité d'interception » est conservé tel quel dans la **Vision produit** (marketing, contexte narratif) mais **n'est jamais utilisé comme exigence technique testable**. Il est remplacé dans les spécifications ci-dessous par des propriétés vérifiables (modèle de menace explicite).

#### 1.4 Analyse Concurrentielle
Tableau conservé sans modification — il relève du positionnement produit, pas d'une spécification technique du prototype "école".

> ➜ **Décision SKELETON [SK]** : Conservé tel quel dans le document de vision, sans valeur d'engagement technique pour le prototype.

---

### 1.2 Vision du Produit "Ecole"

Contenu inchangé (Version SKELETON = prototype + MVP + pièce d'examen RNCP + dépôt public). Section cadrée dans la Vision École.

---

## 2. ARCHITECTURE TECHNIQUE GLOBALE

### 2.1 Schéma Architectural Modulaire

```
Client Qt (Windows / Linux / macOS)
        │
        │  HTTPS (login, refresh)  +  WebSocket sécurisé (messagerie temps réel)
        ▼
   API Gateway  (composant infrastructurel, PAS un 6e microservice métier)
        │  JWT validation + routage
        │  gRPC (protocole interne, transport à préciser en implémentation)
        ├── auth-service
        ├── messaging-service
        ├── files-service
        └── audit-service  
               │
               ▼
        PostgreSQL (instance unique — pas de cluster en SKELETON)
```

> ➜ **Décision SKELETON [SK]** — *(ADR ARC-03)* : L'API Gateway est un **composant infrastructurel distinct** des 5 microservices métier, pas un microservice supplémentaire. Documentation à formuler ainsi : « 5 microservices métier + 1 API Gateway + 1 infrastructure de données ».

> ➜ **Décision SKELETON [??]** — *(ADR ARC-06)* : Le **deploy-service**, tel que décrit dans le cahier des charges (microservice dédié au déploiement), n'est pas implémenté comme service applicatif séparé pour SKELETON. Son rôle est assuré directement par le pipeline CI/CD (GitHub Actions/GitLab CI) et par Docker Compose. → Roadmap V1 si un vrai microservice d'orchestration est jugé pédagogiquement pertinent.

### 2.2 Communication Inter-Services (IPC)

*   **Solution retenue** : gRPC — plus rapide qu'une API REST classique pour les appels interservices.

> ➜ **Décision SKELETON [SK]** — *(ADR ARC-01, décision actée le 16/09/2026)* : Client ↔ Gateway en **HTTP/HTTPS/WebSocket** (autorisé, nécessaire pour un client externe). Gateway ↔ microservices exclusivement en **gRPC** (pas de HTTP interservices, pas de REST consommé directement par les microservices). Le transport bas niveau du gRPC (TCP loopback vs Unix Domain Socket) est un détail d'implémentation à figer en semaine 3, non un point bloquant pour démarrer.

*   **Le C++ natif pour le Back-end**

> ➜ **Décision SKELETON [SK]** — *(ADR CPP-01)* : C++ pour les 5 microservices métier et la Gateway. Les outils périphériques (scripts CI/CD, éventuels outils de test/monitoring) ne sont **pas** contraints au C++.

*   **Le framework Qt pour l'IHM**

> ➜ **Décision SKELETON [SK]** : Qt 6 natif, cible principale de développement/démo Windows + Linux. Par contre, macOS = *best effort*, non garanti faute de machine de test dans l'équipe (à documenter comme limite, pas comme échec).

---

## 3. SPÉCIFICATIONS APPLICATIVES NUMÉROTÉES

### MODULE 1 : MESSAGERIE INSTANTANÉE SÉCURISÉE
  
  

*   **Spécification F1.1 (Conversations privées et collectives)** : 1-to-1 et groupes jusqu'à 100 participants.

> ➜ **Décision SKELETON [SK / RM]** — *(ADR MSG-04)* : Conversations **1-to-1 = SK**, priorité de développement et de démonstration. **Groupes de discussion = RM (V1)** : la complexité de routage et de permissions associée n'est pas justifiée pour une démo de 6 semaines — *à valider collectivement*, cf. matrice UC → composants.
* Ce n'est pas la modélisation qui est complexe — c'est ce qu'elle déclenche en cascade techniquement. Concrètement :
* Fan-out de distribution : un message 1-to-1 a un seul destinataire à router. Un message de groupe doit être dupliqué/distribué vers N destinataires, dont certains hors ligne — il faut une file d'attente par destinataire, pas juste "*un message, une livraison*".
* Permissions dynamiques : qui peut poster dans le groupe change dans le temps (ajout/retrait de membres). Un message envoyé à 14h doit-il rester visible pour quelqu'un retiré du groupe à 15h ? C'est une vraie question de modèle, pas de détail d'implémentation.
* Cohérence de l'historique : si un membre rejoint tardivement, voit-il l'historique avant son arrivée ? Avec du chiffrement E2E, ça veut dire rechiffrer et redistribuer les clés de session de groupe à chaque changement de membre (c'est le problème classique du "group key management" dans les protocoles E2E — Signal, par exemple, y consacre un protocole entier, le Sender Keys).
* Tests : chaque scénario ci-dessus double ou triple le nombre de cas à tester par rapport au 1-to-1.
  
  
*   **Spécification F1.2 (Chiffrement AES-256 E2E)** : contenu chiffré côté émetteur, serveur en routage aveugle.

> ➜ **Décision SKELETON [SK]** — *(ADR SEC-02)* : Pas de « AES-256 partout » générique. Primitives issues d'une bibliothèque éprouvée (**libsodium** retenu, cf. §4), algorithme, mode opératoire et gestion des clés définis dans une **spécification cryptographique dédiée** (document séparé, `Crypto.md`), avant tout développement du module.

*   **Spécification F1.3 (WebSockets sécurisés)** : temps réel via WebSocket.

> ➜ **Décision SKELETON [SK]** — *(ADR ARC-02)* : WebSocket Client → Gateway confirmé. Gateway → messaging-service en gRPC (streaming gRPC pour le push temps réel).

*   **Spécification F1.4 (Présence et notifications sécurisées)**

> ➜ **Décision SKELETON [SK]** — *(ADR MSG-02)* : Présence limitée à 3 états — `ONLINE / OFFLINE / AWAY` —, visibilité contrôlée par permission simple, **pas d'historique de présence**. Notifications remplacées par des règles explicites : nouveau message, mention, alerte, ajout à un groupe d'alerte, avec possibilité de désactivation. « Intelligentes » n'est pas retenu comme exigence (non testable).

*   **Spécification F1.5 (Fils de discussion / Threading)**

> ➜ **Décision SKELETON [SK simplifié / RM]** — *(ADR MSG-01)* : « Thread » = réponse simple à un message ciblé (pas d'arborescence complexe). Un vrai modèle de threading multi-niveaux est reporté en Roadmap.

*   **Spécification F1.6 (Recherche locale chiffrée)**

> ➜ **Décision SKELETON [SK]** : Recherche texte simple sur l'index local déchiffré côté client. Le serveur ne fournit aucun moteur de recherche sur le contenu — confirmé.

*   **Spécification F1.7 (Modèle de threading scalable à 10 000 msg/s)**

> ➜ **Décision SKELETON [DS pour la valeur / SK pour le principe]** — *(ADR PERF-01)* : Le chiffre de 10 000 messages/seconde n'est **pas démontré** sur le matériel de développement de l'équipe. Un scénario de benchmark reproductible est défini (N clients simulés, taille moyenne de message, durée, latence P95/P99, taux d'erreur) et une **valeur cible réaliste** est mesurée et publiée pour la démo, avec la mention explicite : « objectif d'architecture cible : 10 000 msg/s ; valeur mesurée en environnement de démonstration : X msg/s ».
*Imaginons que nous mesurions le temps de livraison de 1000 messages. Une moyenne peut être excellente (50 ms) alors que 50 utilisateurs ont attendu 3 secondes — la moyenne les "noie" dans la masse. Un système MSF où certains messages critiques traînent 3 secondes, c'est un problème réel qu'une moyenne cache.*
  - *P95 (95e percentile) : la valeur en dessous de laquelle se trouvent 95% des mesures. Si P95 = 120 ms, ça veut dire que 95% des messages sont arrivés en moins de 120 ms — et donc que 5% ont mis plus longtemps.*
 - *P99 : la même idée mais pour 99% des mesures — plus strict, ça révèle les pires cas (les 1% de "traînards"), souvent les plus révélateurs d'un problème (surcharge, timeout réseau, etc.).*
  
  
*   **Spécification F1.8 (Priorisation d'urgence médicale)**

> ➜ **Décision SKELETON [SK simplifié]** — *(ADR MSG-03)* : Trois niveaux `NORMAL / HIGH / CRITICAL`, avec alerte visuelle/sonore côté client sur `CRITICAL`. Pas de garantie de livraison temps réel dur (préemption réseau) — la priorité est **logique et d'affichage**, pas une garantie d'infrastructure.

*   **Spécification F1.9 (Diffusion d'alerte globale, accusé de réception forcé)**

> ➜ **Décision SKELETON [SK]** — *(ADR MSG-05, cœur fonctionnel retenu — UC-10/UC-11)* : Diffusion à un **groupe d'alerte** (distinct du groupe de discussion, cf. Vision École). L'« accusé forcé » est réinterprété comme un **suivi d'acquittement** (état visible par l'émetteur : acquitté / non acquitté / relance après délai), pas un blocage technique du destinataire.

---

### MODULE 2 : PARTAGE DE FICHIERS SÉCURISÉ

*   **Spécification F2.1 (Serveur HTTP/HTTPS C++ natif)**

> ➜ **Décision SKELETON [SK]** — *(cohérent avec ADR ARC-01)* : Interface HTTPS externe uniquement (Client/Gateway ↔ Files API), via une **bibliothèque C++ éprouvée** (ex. cpp-httplib ou équivalent) — pas de parsing HTTP développé « maison ». Les échanges Gateway ↔ files-service restent en gRPC.

*   **Spécification F2.2 (Chiffrement streaming et chunking)**

> ➜ **Décision SKELETON [SK]** — *(ADR FILE-03)* : Protocole de transfert minimal défini : `TransferId, ChunkId, TotalChunks, Payload, IntegrityTag`. Reprise après coupure = **best effort simple** (relance du chunk en échec), pas de mécanisme de reprise avancé (retry exponentiel, timeout paramétrable fin) → amélioration en Roadmap.

*   **Spécification F2.3 (Compression zlib)**

> ➜ **Décision SKELETON [SK]** : Ordre fixé — **compression avant chiffrement**, confirmé.

*   **Spécification F2.4 (Virtual File System chiffré « custom »)**

> ➜ **Décision SKELETON [SK simplifié]** — *(ADR SEC-02 étendu)* : Pas de VFS bas niveau développé « maison ». Stockage local chiffré fichier par fichier (AES via libsodium) dans un répertoire dédié, avec un index chiffré simple. Un vrai système de fichiers virtuel est hors périmètre SKELETON.

*   **Spécification F2.5 (Déduplication au niveau des blocs)**

> ➜ **Décision SKELETON [DS]** — *(ADR FILE-02, déjà tranché en Vision École)* : Incompatible en l'état avec le modèle E2E (fichiers chiffrés avec nonces différents = blocs différents). Hors périmètre du prototype. Roadmap uniquement si un mécanisme de déduplication compatible E2E est spécifié.

*   **Spécification F2.6 (Upload 500 Mo < 3 min)**

> ➜ **Décision SKELETON [SK, à condition de séparer les scénarios]** — *(ADR PERF-03)* : Deux scénarios distincts et **jamais présentés comme simultanés** : (1) réseau de démonstration normal → 500 Mo < 3 min démontré ; (2) connexion satellite dégradée 256 kbps → mode dégradé avec reprise, sans contrainte de temps.

*   **Spécification F2.7 (Backup terrain autonome sans Internet)**

> ➜ **Décision SKELETON [SK simplifié]** — *(ADR FILE-01)* : Modèle *Client → stockage local chiffré → file d'attente → synchronisation dès retour réseau*. Testé par simulation (coupure réseau provoquée en environnement de dev), pas en conditions terrain réelles de 48h.

---

### MODULE 3 : AUTHENTIFICATION ET AUTORISATION

*   **Spécification F3.1 (Login MFA, bcrypt optimisé)**

> ➜ **Décision SKELETON [SK]** — *(ADR AUTH-02, interdiction de crypto maison)* : Pas de « bcrypt optimisé » développé en interne — bibliothèque standard (bcrypt ou argon2 via libsodium/OpenSSL). MFA = **TOTP** si le temps le permet, sinon **MFA simulé** pour la démo — mécanisme documenté explicitement dans les deux cas.

*   **Spécification F3.2 (Refresh JWT, validation continue)**

> ➜ **Décision SKELETON [SK]** — *(ADR AUTH-01)* : Bibliothèque JWT standard (jwt-cpp), pas de « JWT custom ». Modèle retenu : *Access Token court + Refresh Token + version de session côté Auth*.

*   **Spécification F3.3 (Validation rapide < 45 ms)**

> ➜ **Décision SKELETON [SK]** — *(ADR PERF-02)* : Métrique précisée avant mesure : latence de `GET /auth/validate` mesurée en **P95/P99**, pas en moyenne seule, sur le scénario de benchmark défini en F1.7. Valeur cible ajustée si nécessaire après première mesure réelle.

*   **Spécification F3.4 (Permissions granulaires)**

> ➜ **Décision SKELETON [SK simplifié]** — *(ADR AUTH-03)* : Matrice **RBAC simple** (rôles fixes : `user`, `coordinateur`, `admin`) plutôt qu'un système de permissions granulaires par ressource/action complet. Matrice complète (User/Role/Permission/Resource/Action/Scope) documentée comme cible Roadmap.

*   **Spécification F3.5 (Révocation immédiate + purge clés locales)**

> ➜ **Décision SKELETON [SK]** — *(ADR AUTH-01)* : « Immédiat » redéfini comme : *aucun nouveau traitement authentifié accepté après propagation de la révocation*. Testé fonctionnellement (le token ne fonctionne plus après révocation) ; un chiffrage de latence < 1 seconde n'est **pas garanti par SLA** en environnement de démonstration non chargé.

*   **Spécification F3.6 (Annuaire à 45 000 utilisateurs)**

> ➜ **Décision SKELETON [DS pour le volume / RM pour l'architecture]** — *(ADR AUTH-04)* : Testé avec un jeu de données simulé réduit (quelques centaines d'utilisateurs). La capacité à 45 000 utilisateurs réels reste un objectif d'architecture cible documenté, non démontré dans SKELETON.

---

### MODULE 4 : INTERFACE UTILISATEUR NATIVE QT

*   **Spécification F4.1 (Compatibilité native multiplateforme)**

> ➜ **Décision SKELETON [SK / RM]** — *(ADR QT-01)* : Build et tests validés sur **Windows et Linux** par l'équipe (postes de développement réels). Build macOS = *best effort non garanti* — documenté comme limite, pas comme non-conformité.

*   **Spécification F4.2 (Démarrage < 2 secondes)**

> ➜ **Décision SKELETON [SK]** — *(ADR QT-02)* : Mesuré de façon informelle (chronométrage manuel ou script simple), pas de suite de benchmark automatisée dédiée en CI pour cette métrique.

*   **Spécification F4.3 (RAM < 50 Mo)**

> ➜ **Décision SKELETON [SK]** — *(ADR QT-02)* : Profilage ponctuel (ex. gestionnaire de tâches / Valgrind massif sous Linux), pas de test de charge mémoire automatisé en pipeline.

*   **Spécification F4.4 (Résilience environnementale extrême — 45 °C, poussière, 256 kbps)**

> ➜ **Décision SKELETON [SK partiel / DS]** — *(ADR QT-03)* : Seule la dimension **réseau dégradé (256 kbps simulé via throttling)** est testable et testée par l'équipe. Les conditions physiques (chaleur, poussière, matériel vieillissant) sont **non démontrables** dans le cadre du projet étudiant — limite documentée explicitement, non contournée par une affirmation non vérifiée.

---

### MODULE 5 : DÉPLOIEMENT ET INFRASTRUCTURE (CI/CD & ORCHESTRATION)

*   **Spécification F5.1 (Pipeline CI/CD automatisée complète)**

> ➜ **Décision SKELETON [SK]** — *(ADR CI-01)* : Pipeline complète (Build → Tests unitaires/intégration → Package Docker → Deploy → Health check) sur un **environnement de démonstration**, pas une production MSF réelle. Déploiement automatique en production à chaque commit évité — pipeline avec étape staging.

*   **Spécification F5.2 (git push → 5 min → production)**

> ➜ **Décision SKELETON [SK]** — *(ADR CI-02)* : Reformulé : *push sur la branche de déploiement → pipeline complète → environnement de démonstration opérationnel en ≤ 5 minutes*. Rendu ainsi testable et mesurable.

*   **Spécification F5.3 (Orchestration Docker Compose)**

> ➜ **Décision SKELETON [SK]** — *(ADR ARC-03 / ARC-05)* : Docker Compose contient au minimum : `gateway, auth-service, messaging-service, files-service, audit-service, postgresql` (le `deploy-service` n'existe pas en tant que conteneur séparé, cf. §2.1). PostgreSQL en instance unique, pas de cluster.

*   **Spécification F5.4 (Logs d'audit chiffrés inaltérables)**

> ➜ **Décision SKELETON [SK]** — *(ADR AUD-01)* : L'audit-service enregistre des **événements de sécurité et d'administration** (`USER_LOGIN`, `TOKEN_REVOKED`, `FILE_UPLOADED`, `PERMISSION_CHANGED`, ...), **jamais** le contenu déchiffré des messages/fichiers, ni mots de passe, tokens ou clés privées. Asynchrone, hors du chemin critique des requêtes.

---

## 4. STRATÉGIE DE TESTS ET SCÉNARIOS DE CRISE MSF

La philosophie **« Microservices = Complexité × 5 = Tests × 10 »** est conservée comme principe directeur.

> ➜ **Décision SKELETON [SK]** — *(ADR TEST-01)* : « Couverture 100 % » est redéfinie avant tout développement comme : 100 % des **cas d'usage retenus pour SKELETON** couverts par au moins un test d'intégration, et non 100 % des lignes de code. Métrique de couverture de lignes suivie à titre indicatif (outil de coverage C++), sans objectif chiffré contraignant.

### 4.1 Scénarios de Crise Contextuels MSF

Les 5 scénarios du cahier des charges sont conservés comme **scénarios de test cibles**, avec ajustement de réalisme :

*   **Scénario 1 — Connexion dégradée (Auth)** → **[SK]** simulé par throttling réseau local (256 kbps), pas par une vraie liaison satellite.
*   **Scénario 2 — Révocation d'urgence** → **[SK]** fonctionnel (le token ne fonctionne plus), sans garantie chiffrée de latence < 1 s sous charge réelle *(ADR AUTH-01)*.
*   **Scénario 3 — Transfert imagerie médicale 500 Mo** → **[SK]** sur réseau de démonstration normal uniquement *(ADR PERF-03, cf. F2.6)*.
*   **Scénario 4 — Isolation réseau totale (Backup terrain)** → **[SK simplifié]**, coupure simulée en dev, pas 48h en conditions réelles *(ADR FILE-01)*.
*   **Scénario 5 — Urgence chirurgicale, routage < 50 ms sous 10 000 msg/s** → **[DS pour la charge / SK pour le principe]** : le routage prioritaire est démontré fonctionnellement (message `CRITICAL` traité avant les autres dans la file), mais **pas sous une charge réelle de 10 000 msg/s** *(cf. ADR PERF-01)*.

### 4.2 Analyse des Points de Défaillance Critiques (FMEA)

> ➜ **Décision SKELETON [SK]** : Le tableau FMEA est conservé comme grille d'analyse — utile pour prioriser les tests d'intégration — mais toutes les stratégies de résilience listées (réplication de base de données, routage redondant des microservices) sont à annoter individuellement `[SK]/[RM]/[DS]` avant d'être promises comme livrable. En l'état, pour SKELETON : smart pointers C++ et gestion d'erreurs de base = **[SK]** ; réplication PostgreSQL et routage redondant = **[RM]**, cohérent avec ADR ARC-04/ARC-05 (PostgreSQL simple en V1).

---

## 5. EXIGENCES NON-FONCTIONNELLES ET LIVRABLES

### 5.1 Critères d'Acceptation de Performance et de Sécurité

*   **Performance réseau 10 000 msg/s** → **[DS pour la valeur, SK pour le protocole de mesure]** *(ADR PERF-01)* : benchmark reproductible défini et exécuté, valeur mesurée publiée telle quelle (pas nécessairement 10 000).
*   **Temps de réponse < 100 ms** → **[SK]** *(ADR PERF-02)* : mesuré en P95/P99 sur le trajet Client A → réception Client B, en conditions nominales du benchmark.
*   **Disponibilité 99,9 %** → **[SK reformulé]** *(ADR OPS-01)* : « zéro tolérance à la panne » supprimé des exigences testables. Remplacé par : absence de point unique de défaillance évident dans l'architecture applicative + reprise contrôlée après un crash simple d'un service (redémarrage automatique du conteneur). 99,9 % documenté comme **objectif d'architecture cible**, non mesuré sur une période longue avec le prototype.
*   **Audit de sécurité E2E par organisme tiers indépendant** → **[DS]** *(ADR TEST-02)* : hors capacité et hors budget d'un projet étudiant. Remplacé par une **revue cryptographique interne documentée** (choix de primitives, analyse des menaces, tests, documentation) — mention explicite dans le livrable final : « audit externe professionnel non réalisé dans le cadre du projet ».

### 5.2 Livrables Contractuels

> ➜ **Décision SKELETON [SK]** : Les 4 livrables techniques et les 3 livrables fonctionnels du cahier des charges sont conservés en nature, mais **recalibrés à l'échelle SKELETON** :
> 1. Codes sources des 5 microservices + Gateway, conteneurisés → **[SK]**
> 2. Client Qt compilable Windows + Linux (macOS best effort) → **[SK/RM]**
> 3. Suite de tests automatisés couvrant les UC retenus (cf. TEST-01) → **[SK]**
> 4. Pipeline CI/CD documentée et opérationnelle sur environnement de démonstration → **[SK]**
> 5. « Plateforme pleinement déployée et opérationnelle » pour MSF → **[DS]**, remplacé par « démonstrateur fonctionnel validé en environnement pédagogique »
> 6. Formation des équipes MSF → **[DS]**, hors périmètre (pas de client réel)
> 7. Procédures de déploiement et plan de maintenance → **[SK]**, rédigés pour l'environnement de démonstration

---

## 6. TABLEAU DE SYNTHÈSE — TRAÇABILITÉ SPÉCIFICATION → DÉCISION

| Spec | Sujet | ADR | Statut |
| :--- | :--- | :--- | :--- |
| F1.1 | Groupes 100 participants | MSG-04 | SK (1-to-1) / RM (groupes) |
| F1.2 | AES-256 E2E | SEC-02 | SK |
| F1.3 | WebSocket | ARC-02 | SK |
| F1.4 | Présence/notifications | MSG-02 | SK |
| F1.5 | Threading | MSG-01 | SK simplifié / RM |
| F1.6 | Recherche locale | — | SK |
| F1.7 | 10 000 msg/s | PERF-01 | DS (valeur) / SK (méthode) |
| F1.8 | Priorité message | MSG-03 | SK simplifié |
| F1.9 | Diffusion alerte | MSG-05 | SK |
| F2.1 | Serveur HTTP/HTTPS | ARC-01 | SK |
| F2.2 | Streaming/chunking | FILE-03 | SK |
| F2.3 | Compression zlib | — | SK |
| F2.4 | VFS chiffré custom | SEC-02 | SK simplifié |
| F2.5 | Déduplication blocs | FILE-02 | DS |
| F2.6 | 500 Mo / 3 min | PERF-03 | SK (2 scénarios séparés) |
| F2.7 | Backup terrain | FILE-01 | SK simplifié |
| F3.1 | Login MFA / bcrypt | AUTH-02 | SK |
| F3.2 | Refresh JWT | AUTH-01 | SK |
| F3.3 | Validate < 45 ms | PERF-02 | SK |
| F3.4 | Permissions granulaires | AUTH-03 | SK simplifié (RBAC) |
| F3.5 | Révocation immédiate | AUTH-01 | SK |
| F3.6 | 45 000 utilisateurs | AUTH-04 | DS (volume) / RM (archi) |
| F4.1 | Multiplateforme | QT-01 | SK (Win/Linux) / RM (macOS) |
| F4.2 | Démarrage < 2s | QT-02 | SK |
| F4.3 | RAM < 50 Mo | QT-02 | SK |
| F4.4 | Résilience environnementale | QT-03 | SK partiel / DS |
| F5.1 | Pipeline CI/CD | CI-01 | SK |
| F5.2 | 5 min production | CI-02 | SK |
| F5.3 | Docker Compose | ARC-03/05 | SK |
| F5.4 | Audit chiffré | AUD-01 | SK |
| §5.1 | 10 000 msg/s (NFR) | PERF-01 | DS |
| §5.1 | < 100 ms (NFR) | PERF-02 | SK |
| §5.1 | 99,9 % uptime | OPS-01 | SK reformulé |
| §5.1 | Audit tiers | TEST-02 | DS |

---

**Prochaine étape proposée** : cette table de traçabilité devient la base de la **matrice Acteurs → UC → Exigences → Composants techniques** évoquée en Vision École — chaque ligne SK doit ensuite être reliée à un cas d'utilisation du catalogue `UseCases.md` et à une entité du `Domain.md`.


---

# UseCases

> Draft V0.1 qui est en construction / correction /évolution
> Premières réflexions

# Cas d’utilisation — Application cliente SECURECLOUD


## UC-01 — Se connecter au serveur

**Objectif :**
Permettre à un utilisateur authentifié d’établir une session avec le serveur SECURECLOUD.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'application cliente est installée sur le poste de l'utilisateur.
- L'utilisateur dispose d'un compte SECURECLOUD actif.
- Le serveur SECURECLOUD est accessible.

**Scénario nominal :**
1. L'utilisateur ouvre l'application cliente.
2. L'application affiche l'écran de connexion.
3. L'utilisateur saisit son identifiant et son mot de passe.
4. L'utilisateur valide la connexion.
5. Le serveur vérifie les informations d'authentification.
6. Le serveur accepte l'authentification.
7. L'application établit la session utilisateur.
8. L'application affiche l'interface principale.

**Scénarios alternatifs / exceptions :**
- Les identifiants sont incorrects → l'application indique l'échec de l'authentification et permet une nouvelle tentative.
- Le réseau est indisponible → l'application indique que le serveur est inaccessible.
- Le compte n'est pas autorisé à se connecter → l'application indique le refus de connexion.
- L'utilisateur a oublié son identifiant → il utilise la procédure de récupération prévue.
- L'utilisateur a oublié son mot de passe → il utilise la procédure de récupération/réinitialisation prévue.

**Postconditions :**
- **Succès :*- une session utilisateur authentifiée est établie.
- **Échec :*- aucune session authentifiée n'est établie.

---

## UC-02 — Envoyer un message individuel

**Objectif :**
Permettre à un utilisateur d'envoyer un message à un autre utilisateur SECURECLOUD.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- Le destinataire possède un compte SECURECLOUD actif.
- Une connexion au service SECURECLOUD est disponible.

**Scénario nominal :**
1. L'utilisateur sélectionne un destinataire.
2. L'application ouvre la conversation individuelle.
3. L'utilisateur rédige son message.
4. L'utilisateur valide l'envoi.
5. L'application transmet le message au service SECURECLOUD.
6. Le message est distribué au destinataire.
7. Le message apparaît dans le fil de conversation.

**Scénarios alternatifs / exceptions :**
- Le destinataire n'est plus disponible → le message est traité selon le mécanisme de livraison différée prévu.
- La connexion est interrompue → l'application signale l'échec ou conserve le message pour une nouvelle tentative, selon les règles définies.
- Le message ne peut pas être transmis → l'application indique son état à l'utilisateur.

**Postconditions :**
- **Succès :*- le message est accepté par le service et associé à la conversation.
- **Échec :*- l'utilisateur est informé que le message n'a pas pu être transmis.

---

## UC-03 — Envoyer un message dans un groupe

**Objectif :**
Permettre à un utilisateur d'envoyer un message à l'ensemble des membres autorisés d'un groupe de discussion.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- Le groupe existe.
- L'utilisateur est membre du groupe.
- L'utilisateur dispose du droit d'envoyer des messages dans le groupe.

**Scénario nominal :**
1. L'utilisateur sélectionne un groupe.
2. L'application ouvre la conversation du groupe.
3. L'utilisateur rédige son message.
4. L'utilisateur valide l'envoi.
5. L'application transmet le message au service SECURECLOUD.
6. Le service distribue le message aux membres autorisés du groupe.
7. Le message apparaît dans le fil de discussion du groupe.

**Scénarios alternatifs / exceptions :**
- L'utilisateur n'est plus membre du groupe → l'envoi est refusé.
- L'utilisateur ne possède pas les droits nécessaires → l'envoi est refusé.
- Un ou plusieurs membres sont momentanément indisponibles → la livraison est traitée selon le mécanisme prévu.
- La connexion est interrompue → l'application indique l'état du message et applique le mécanisme de reprise prévu.

**Postconditions :**
- **Succès :*- le message est associé au groupe et distribué aux membres autorisés.
- **Échec :*- l'utilisateur est informé du refus ou de l'échec de transmission.

---

## UC-04 — Joindre une pièce jointe à un message

**Objectif :**
Permettre à un utilisateur d'associer un fichier à un message avant son envoi.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- Une conversation individuelle ou un groupe est sélectionné.
- Le fichier respecte les règles de taille et de format définies par SECURECLOUD.

**Scénario nominal :**
1. L'utilisateur sélectionne l'option d'ajout d'une pièce jointe.
2. L'application lui permet de sélectionner un fichier.
3. L'utilisateur sélectionne le fichier.
4. L'application vérifie les contraintes applicables au fichier.
5. Le fichier est associé au message.
6. L'utilisateur valide l'envoi.
7. Le fichier et le message sont transmis selon les mécanismes de sécurité prévus.
8. Le destinataire ou les membres autorisés peuvent récupérer la pièce jointe.

**Scénarios alternatifs / exceptions :**
- Le fichier dépasse la taille maximale → l'application refuse la pièce jointe.
- Le format du fichier n'est pas autorisé → l'application refuse la pièce jointe.
- Le fichier n'est plus accessible → l'application demande à l'utilisateur de le sélectionner à nouveau.
- La transmission échoue → l'application indique l'état de la pièce jointe.

**Postconditions :**
- **Succès :*- la pièce jointe est associée au message et transmise aux destinataires autorisés.
- **Échec :*- la pièce jointe n'est pas transmise.

---

## UC-05 — Consulter ses conversations

**Objectif :**
Permettre à l'utilisateur de consulter ses conversations individuelles et de groupe.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.

**Scénario nominal :**
1. L'utilisateur accède à sa liste de conversations.
2. L'application affiche les conversations auxquelles il participe.
3. L'utilisateur sélectionne une conversation.
4. L'application affiche l'historique accessible de la conversation.

**Postconditions :**

- L'utilisateur peut consulter les messages auxquels il est autorisé à accéder.

---

## UC-06 — Recevoir un message

**Objectif :**
Permettre à l'utilisateur de recevoir et consulter les messages qui lui sont destinés.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur dispose d'une session active ou d'un mécanisme de réception prévu par le système.

**Scénario nominal :**
1. Le service SECURECLOUD reçoit un message destiné à l'utilisateur.
2. Le service transmet le message à l'application cliente.
3. L'application reçoit le message.
4. L'application l'intègre à la conversation correspondante.
5. L'application informe l'utilisateur de la réception.

**Postconditions :**

- Le message reçu est disponible dans la conversation concernée.

---

## UC-07 — Gérer les conversations et groupes

**Objectif :**
Permettre à l'utilisateur autorisé de créer et gérer des groupes de discussion.

**Acteur principal :**Utilisateur autorisé

**Préconditions :**
- L'utilisateur est authentifié.
- L'utilisateur dispose des droits nécessaires.

**Scénario nominal :**
1. L'utilisateur crée un groupe ou sélectionne un groupe qu'il administre.
2. L'utilisateur définit ou modifie les paramètres du groupe.
3. L'utilisateur ajoute ou retire des membres selon ses droits.
4. L'application transmet les modifications au service SECURECLOUD.
5. Le groupe est mis à jour.

**Postconditions :**

- La configuration du groupe correspond aux modifications validées.

---

## UC-08 — Télécharger / consulter une pièce jointe

**Objectif :**
Permettre à un utilisateur autorisé de récupérer une pièce jointe reçue.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur est authentifié.
- La pièce jointe appartient à une conversation à laquelle l'utilisateur a accès.

**Scénario nominal :**
1. L'utilisateur sélectionne une pièce jointe.
2. L'application vérifie que l'utilisateur est autorisé à y accéder.
3. La pièce jointe est récupérée.
4. L'application permet à l'utilisateur de la consulter ou de l'enregistrer.

**Postconditions :**
- Le fichier est accessible à l'utilisateur conformément à ses droits.

---

## UC-09 — Se déconnecter

**Objectif :**
Permettre à l'utilisateur de terminer sa session SECURECLOUD.

**Acteur principal :**
Utilisateur

**Préconditions :**
- L'utilisateur possède une session active.

**Scénario nominal :**
1. L'utilisateur demande la déconnexion.
2. L'application termine la session.
3. L'application revient à l'écran de connexion.

**Postconditions :**
- La session utilisateur n'est plus active.
- Les fonctionnalités nécessitant une authentification ne sont plus accessibles.

---


LES ALERTES :
Fonctionnellement, une alerte possède une logique propre : destinataires, niveau de priorité, diffusion, accusé de réception et éventuellement suivi de l’état.
Je déconseille de mélanger « groupe de discussion » et « groupe d’alerte ». Un groupe de discussion sert à échanger ; un groupe d’alerte définit une population à laquelle une alerte critique peut être diffusée.*
Je retiendrais donc la notion de **groupe d'alerte**.


## UC-10 — Gérer les alertes prioritaires

**Objectif :**
Permettre à un utilisateur autorisé de créer, diffuser et suivre des alertes prioritaires destinées à un ensemble défini d'utilisateurs.

**Acteurs principaux :**
 - Utilisateur autorisé à émettre des alertes

**Préconditions :**
 - L'émetteur est authentifié.
 - L'émetteur dispose du droit d'émettre des alertes.
 - Les destinataires ou le groupe d'alerte existent.
 - Le service SECURECLOUD est disponible.

**Scénario nominal :**
1. L'émetteur sélectionne la fonction « Nouvelle alerte ».
2. Il définit le niveau de priorité de l'alerte.
3. Il saisit le contenu de l'alerte ou utilise un catalogue pré-défini.
4. Il sélectionne la cible de diffusion (si elle n'est pas pré-définie) :
   * un groupe d'alerte ;
   * éventuellement l'ensemble des utilisateurs autorisés.
5. L'émetteur valide la diffusion.
6. SECURECLOUD distribue l'alerte aux destinataires concernés.

**Scénarios alternatifs / exceptions :**
 - L'émetteur ne possède pas les droits nécessaires → il ne peut sélectionner cette option.
 - Le groupe d'alerte n'existe plus ou n'est pas accessible → la diffusion est refusée.
 - Certains destinataires sont momentanément indisponibles → l'alerte est conservée pour une distribution ultérieure selon les règles définies.
 - La transmission échoue → l'émetteur est informé de l'état de diffusion.

**Postconditions :**
 - **Succès :** l'alerte est enregistrée et transmise aux destinataires concernés.
 -  **Échec :** l'émetteur est informé que l'alerte n'a pas été diffusée conformément à sa cible.


## UC-11 — Recevoir une alerte

**Objectif :**
Permettre à un utilisateur du groupe d'alerte de recevoir une alerte et de l'acquitter.

**Acteurs principaux :**
 - Utilisateur du groupe d'alertes

**Préconditions :**
 - L'utilisateur est connecté
 - Le groupe d'alerte existent.
 - Le service SECURECLOUD est disponible.

**Scénario nominal :**
1. L'utilisateur reçois une alerte
2. L'alerte est visible et accessible immédiatement
3. L'utilisateur accède à l'alerte
4. L'utilisateur accède à l'alerte et acquitte
5. SECURECLOUD enregistre les acquittements

**Scénarios alternatifs / exceptions :**
 - L'utilisateur n'a pas lu l'alerte : alarme au bout d'un temps donné
 - L'utilisateur n'est pas connecté au moment de l'envoi de l'alerte : elle s'affiche dés la connexion.

**Postconditions :**
 - **Lu et acquitté :** l'alerte est notifié comme acquittement par l'utilisateur
 - **Non délivrée :** l'alerte sera notifiée dés que les conditions de connexion le permettent



## A voir pour plus tard :

### UC-12 — Gérer les groupes d'alerte ??? (V2?)

Je créerais également un cas distinct, car la gestion de la population destinataire est une fonction métier à part entière.

**Objectif :**
Permettre à un utilisateur autorisé de créer et administrer des groupes d'alerte.

**Acteur principal :**
Administrateur ou utilisateur disposant des droits appropriés.

**Préconditions :**
* L'utilisateur est authentifié.
* L'utilisateur possède les droits d'administration des groupes d'alerte.

**Scénario nominal :**

1. L'utilisateur crée un groupe d'alerte.
2. Il définit son nom et ses paramètres.
3. Il ajoute ou retire des membres.
4. Il valide la configuration.
5. Le système enregistre le groupe.

**Exemple :**

```text
Groupe d'alerte : Mission Palestine

Membres :
 ├── Utilisateur A
 ├── Utilisateur B
 ├── Utilisateur C
 └── Utilisateur D

Alertes reçues :
 ├── Évacuation
 ├── Danger imminent
 ├── Arrestation
 └── Perte de contact
```

### Un point architectural important

Je distinguerais clairement trois concepts :

```text
Utilisateur
    │
    ├──────────────► Conversation 1-to-1
    │
    ├──────────────► Groupe de discussion
    │
    └──────────────► Groupe d'alerte
                         │
                         ├── Utilisateur A
                         ├── Utilisateur B
                         └── Utilisateur C
```

Un utilisateur peut donc appartenir à plusieurs groupes de discussion **et** plusieurs groupes d'alerte.

Une alerte n'est pas nécessairement une conversation. Elle doit avoir son propre cycle de vie :

```text
Création
   ↓
Validation
   ↓
Diffusion
   ↓
Réception
   ↓
Prise en compte
   ↓
Clôture
```

Cela permet notamment de distinguer :

* **alerte envoyée**
* **alerte reçue**
* **alerte lue**
* **alerte acquittée**
* **alerte non acquittée**
* **alerte clôturée**

Pour un système destiné à des équipes terrain, l'**acquittement** est particulièrement intéressant. Par exemple, une alerte « ÉVACUATION IMMÉDIATE » pourrait exiger une confirmation de chaque membre du groupe. L'émetteur pourrait alors voir :

```text
ÉVACUATION IMMÉDIATE
Groupe : Mission Palestine

✓ Membre A — acquitté
✓ Membre B — acquitté
⚠ Membre C — non acquitté
✓ Membre D — acquitté
```

Pour formaliser la gestion des alertes

* Construire le diagramme UML des alertes
* Rédiger les exigences fonctionnelles des alertes


## Remarque importante sur le niveau d'abstraction

« joindre une pièce jointe à un message » peut être modélisé comme une extension de **Envoyer un message**, plutôt que comme une fonction totalement indépendante :

```text
Envoyer un message
       │
       ├── <<include>> Authentification
       │
       └── <<extend>> Joindre une pièce jointe
```

Il faut aussi prévoir : "télécharger une pièce jointe"

Pour le projet SECURECLOUD, je partirais donc sur un catalogue fonctionnel d'environ **8 à 12 cas d'utilisation majeurs**, puis je ferais une matrice :

**Acteurs → Cas d'utilisation → Exigences fonctionnelles → exigences de sécurité → composants techniques.**

C'est beaucoup plus propre pour le dossier RNCP et permettra ensuite de dériver directement le diagramme UML des cas d'utilisation. Le codage ne sera plus qu'une étape de "mise en code".

Je vais structurer `UseCases.md` comme un document de spécification fonctionnelle : périmètre, acteurs, conventions, puis catalogue exhaustif des cas d’utilisation avec préconditions, scénario nominal, alternatives/exceptions et postconditions.

Pour finaliser UseCases.md, il faut le confronter aux exigences fonctionnelles.
	

---

# DOMAIN.md — SECURECLOUD[SKELETON]

> Modèle du domaine métier. Ce document précède et gouverne l'architecture technique.
> Toute décision d'architecture qui contredit une règle définie ici doit revenir ici d'abord — pas l'inverse.
> Toutefois, il est susceptible d'évoluer par vitération.
> **Convention linguistique** (à partir de cette itération) : les concepts et les identifiants du domaine sont nommés en **anglais** (nom canonique, celui qui apparaîtra dans le code — classes, méthodes, commentaires). Les définitions, explications et discussions restent en **français**. Le nom anglais est indiqué entre parenthèses à chaque première occurrence d'un terme et dans le glossaire §2.

---

## 0. OBJET ET PÉRIMÈTRE DE CE DOCUMENT

Ce document répond à une seule question : **de quoi parle SecureCloud, indépendamment de toute technologie ?**

Il ne contient **aucune** mention de Qt, gRPC, WebSocket, Docker, PostgreSQL, libsodium ou C++. Si l'un de ces mots apparaît dans une phrase de définition, c'est le signe que la phrase appartient à `Architecture.md`, pas ici.

Il s'appuie sur :
- le catalogue de cas d'utilisation (`UseCases.md`),
- le cahier des spécifications (`specifications-securecloud-skeleton.md`) et ses décisions SK/RM/DS.

Chaque entité ci-dessous porte son **statut SKELETON** (SK / RM / DS) pour rester traçable avec la spec.

---

## 1. UNE PARTICULARITÉ STRUCTURANTE : DEUX POINTS DE VUE SUR LE MÊME DOMAINE

Avant le glossaire, une règle qui conditionne tout le reste (décision **SEC-03**) :

Le **contenu** (texte d'un message, octets d'un fichier) n'a pas la même réalité selon où on se place :

- **Côté Client** : le contenu existe en clair. C'est le seul endroit du système où le domaine métier "voit" vraiment ce qui est écrit.
- **Côté Serveur** (Gateway, microservices) : le contenu n'existe jamais autrement que comme **donnée opaque**. Le serveur manipule des enveloppes, pas des messages lisibles.

Conséquence directe pour ce document : quand on décrit l'entité **Message**, l'attribut *contenu* a deux visages :
- `PlaintextContent` (vu uniquement par le Client)
- `OpaqueContent` (vu par le Serveur — un bloc de données sans signification métier pour lui)

Ce n'est pas un détail de chiffrement, c'est une **frontière de domaine**. On y reviendra à chaque entité concernée.

---

## 2. VOCABULAIRE UBIQUITAIRE

| Terme (FR) | Nom canonique (EN) | Définition | Ce que ce n'est PAS |
|---|---|---|---|
| Utilisateur | `User` | Personne physique disposant d'un compte SecureCloud, identifiée de façon unique dans le système. | Pas un rôle — un rôle est un attribut de l'utilisateur, pas l'utilisateur lui-même. |
| Session | `Session` | Fenêtre de temps pendant laquelle un Utilisateur est reconnu comme authentifié par le système. | Pas un token — la session est le concept métier ; le token est son implémentation technique. |
| Conversation individuelle | `DirectConversation` | Échange de Messages entre exactement deux Utilisateurs. | Pas un Groupe de discussion. |
| Groupe de discussion | `GroupConversation` | Échange de Messages entre plusieurs Utilisateurs, à but conversationnel. *(RM — implémentation reportée, cf. §4.4)* | Pas un Groupe d'alerte : pas de notion de diffusion d'urgence ni d'acquittement. |
| Groupe d'alerte | `AlertGroup` | Ensemble nommé d'Utilisateurs constituant une cible de diffusion pour une Alerte. | Pas un Groupe de discussion : on n'y échange pas, on y diffuse et on y acquitte. |
| Message | `Message` | Unité de communication envoyée par un Utilisateur dans une Conversation, avec un contenu et un niveau de priorité. | Pas une Alerte. |
| Alerte | `Alert` | Communication à diffusion large, émise par un Utilisateur autorisé vers un Groupe d'alerte, suivie individuellement jusqu'à acquittement. | Pas un Message prioritaire. |
| Acquittement | `Acknowledgment` | Confirmation individuelle, par un membre d'un Groupe d'alerte, qu'il a pris connaissance d'une Alerte. | Pas un simple "lu". |
| Pièce jointe | `Attachment` | Fichier associé à un Message, transmis et stocké de façon chiffrée. | Pas un Message. |
| Permission | `Permission` | Autorisation accordée à un Utilisateur (via son Rôle) d'effectuer une action sur une ressource donnée. | Pas un Rôle. |
| Rôle | `Role` | Catégorie fixe assignée à un Utilisateur, déterminant ses Permissions. *(SK simplifié — RBAC à 3 rôles)* | Pas une permission granulaire par ressource individuelle. |
| Événement d'audit | `AuditEvent` | Trace d'une action de sécurité ou d'administration, jamais du contenu métier. | Pas un log applicatif générique. |
| Présence | `PresenceStatus` | État de disponibilité déclaré d'un Utilisateur. *(SK : `ONLINE` / `OFFLINE` uniquement — voir §4.1)* | Pas un historique de connexion. |

---

## 3. CARTE DES ENTITÉS

```
User
   │
   ├──► Session (authentification active)
   │
   ├──► DirectConversation ◄──► User (2)
   │         │
   │         └──► Message ──► Attachment (0..n)
   │
   ├──► GroupConversation [RM — interface anticipée dès SKELETON, cf. §4.4] ◄──► User (n)
   │         │
   │         └──► Message
   │
   └──► AlertGroup ◄──► User (n, membres)
             │
             └──► Alert ──► Acknowledgment (1 par membre destinataire)

Role ──► Permission (n)
User ──► Role (1)

AuditEvent : observe des actions sur User / Session / Alert / Attachment
             (jamais le contenu d'un Message)
```

---

## 4. FICHES D'ENTITÉS

### 4.1 Utilisateur (`User`) — **[SK]**

**Définition métier** : une personne autorisée à utiliser SecureCloud.

**Ce qui le caractérise, conceptuellement** (pas un schéma de base de données) :
- un identifiant unique et stable
- un rôle (`Role`, parmi les 3 retenus)
- une clé publique, nécessaire pour que d'autres Utilisateurs puissent lui envoyer du contenu chiffré
- un état de présence courant (`PresenceStatus`)

**Invariants** :
- Un Utilisateur possède toujours exactement un Rôle (jamais zéro, jamais plusieurs — simplification SK, cf. AUTH-03).
- Un Utilisateur sans Session active ne peut engager aucune action nécessitant une identité vérifiée (envoyer un Message, émettre une Alerte, etc.).

**Présence (`PresenceStatus`) — précision suite à arbitrage** :
- **SKELETON [SK]** : deux valeurs seulement — `ONLINE` / `OFFLINE`. Bascule automatique liée à l'état de la Session (une Session active = `ONLINE`, une Session terminée ou absente = `OFFLINE`). Pas de bascule manuelle par l'utilisateur en SKELETON.
- **`AWAY` [RM V1]** : reporté, décision actée (arbitrage du 16/09/2026). Reste dans le glossaire comme valeur future du même concept (`PresenceStatus`), pas comme un concept séparé — quand elle sera implémentée, ce sera une troisième valeur du même type, pas un nouveau modèle.

**Ce qu'il n'est pas** : l'Utilisateur n'est pas sa Session — un même Utilisateur peut avoir eu plusieurs Sessions successives dans le temps.

---

### 4.2 Session (`Session`) — **[SK]**

**Définition métier** : la reconnaissance, par le système, qu'un Utilisateur est actuellement authentifié.

**Invariants** :
- Une Session a toujours un Utilisateur associé.
- Une Session peut être **révoquée** : après révocation, plus aucune action ne peut être accomplie au nom de cette Session, même si elle n'avait pas atteint son terme naturel (décision AUTH-01).
- Une Session révoquée ne redevient jamais active.

**Cycle de vie** : voir §5.1.

---

### 4.3 Conversation individuelle (`DirectConversation`) — **[SK]**

**Définition métier** : le cadre dans lequel deux Utilisateurs échangent des Messages.

**Invariants** :
- Une `DirectConversation` relie toujours exactement deux Utilisateurs, jamais plus (sinon c'est un `GroupConversation`).
- Une `DirectConversation` existe dès le premier Message envoyé entre les deux Utilisateurs — elle n'a pas besoin d'être "créée" explicitement au préalable.

---

### 4.4 Groupe de discussion (`GroupConversation`) — **[RM — comportement reporté, interface anticipée en SKELETON]**

**Définition métier** : le cadre dans lequel plusieurs Utilisateurs échangent des Messages, avec une notion d'appartenance et éventuellement d'administration.

**Précision suite à arbitrage** : le comportement complet de `GroupConversation` (fan-out de distribution, permissions dynamiques par membre, gestion des clés de groupe pour le chiffrement E2E — cf. échange précédent sur la complexité réelle) reste **Roadmap V1**. Mais la **conception des classes et interfaces** du domaine doit anticiper son arrivée dès SKELETON :

> **Directive pour `Architecture.md`** : `DirectConversation` et `GroupConversation` doivent être modélisées comme deux variantes concrètes d'un même concept abstrait — par exemple une interface/classe de base commune `Conversation` portant les opérations génériques (`addMessage`, `listParticipants`, `history`), dont `GroupConversation` peut être une implémentation partielle ou différée (méthode virtuelle explicitement non implémentée, avec commentaire renvoyant à cette section et à l'ADR MSG-04, plutôt qu'un silence ou un `TODO` isolé). L'objectif : que l'arrivée du Groupe de discussion en V1 soit une *extension*, jamais une *réécriture* du modèle de Conversation.

Ce point est un pont explicite entre `Domain.md` (le concept) et `Architecture.md` (sa traduction en classes) — c'est la seule dérogation volontaire à la frontière du §7, parce que la décision d'équipe porte justement sur la forme que doit prendre cette anticipation.

---

### 4.5 Message (`Message`) — **[SK]**

**Définition métier** : une unité de communication, écrite par un Utilisateur, adressée à une Conversation.

**Attributs conceptuels** :
- expéditeur (`sender`, un `User`)
- conversation cible (`conversation`)
- contenu — **double visage**, cf. §1 (`PlaintextContent` côté Client / `OpaqueContent` côté Serveur)
- niveau de priorité (`priority`) : `NORMAL`, `HIGH`, `CRITICAL` (F1.8)
- horodatage (`timestamp`)
- statut de livraison (`deliveryStatus`)

**Invariants** :
- Un Message n'existe jamais sans expéditeur ni conversation cible.
- Le `PlaintextContent` d'un Message n'est **jamais** une donnée que le domaine "Serveur" est autorisé à connaître ou à dériver. C'est un invariant de sécurité, pas seulement une contrainte technique.
- La `priority` d'un Message est une information visible du Serveur (elle sert au routage) — elle **n'est pas** couverte par la confidentialité E2E, contrairement au contenu.

**Ce qu'il n'est pas** : le Message n'est pas l'Alerte (`Alert`) — voir §4.8.

---

### 4.6 Pièce jointe (`Attachment`) — **[SK simplifié]**

**Définition métier** : un fichier associé à un Message.

**Attributs conceptuels** :
- fichier associé à exactement un `Message`
- statut de transfert (`transferStatus`)
- contenu — même double visage que le Message (`PlaintextContent` / `OpaqueContent`)

**Invariants** :
- Un `Attachment` est toujours rattaché à un `Message` existant, jamais autonome.
- Deux `Attachment` distincts ne sont jamais reconnus comme "identiques" par le Serveur, même si leur contenu clair l'est (conséquence de FILE-02 — pas de déduplication en SKELETON).

---

### 4.7 Groupe d'alerte (`AlertGroup`) — **[SK]**

**Définition métier** : une population nommée d'Utilisateurs, définie à l'avance, pouvant être la cible d'une diffusion d'Alerte.

**Invariants** :
- Un `AlertGroup` a toujours un nom et au moins un membre pour être utilisable comme cible de diffusion.
- L'appartenance à un `AlertGroup` est indépendante de l'appartenance à un `GroupConversation` — ce sont deux notions d'appartenance distinctes, même si elles peuvent réunir les mêmes personnes dans les faits.

---

### 4.8 Alerte (`Alert`) — **[SK]**

**Définition métier** : une communication à diffusion large et suivie, émise par un Utilisateur autorisé, destinée à un `AlertGroup`, dont la réception doit être individuellement confirmée.

**Attributs conceptuels** :
- émetteur (`emitter`, un `User` autorisé)
- cible (`targetGroup`, un `AlertGroup`)
- niveau de priorité (`priority`)
- contenu (`content`)
- état global (`status`, voir cycle de vie §5.2)
- ensemble d'`Acknowledgment`, un par membre destinataire au moment de la diffusion

**Invariants** :
- Une `Alert` cible toujours un `AlertGroup` déterminé au moment de la diffusion — la liste des destinataires est figée à cet instant.
- Émettre une `Alert` requiert la Permission dédiée (`ALERT_EMIT`).
- Une `Alert` ne peut pas exister sans au moins un `Acknowledgment` attendu (donc sans `AlertGroup` non vide).

---

### 4.9 Acquittement (`Acknowledgment`) — **[SK]**

**Définition métier** : la trace qu'un membre destinataire d'une Alerte en a pris connaissance et l'a explicitement confirmé.

**Invariants** :
- Un `Acknowledgment` est toujours associé à exactement une `Alert` et un `User`.
- Un `Acknowledgment`, une fois posé, est définitif (pas de "dé-acquittement").
- Tant qu'un `Acknowledgment` n'est pas posé, le statut du destinataire correspondant reste `NOT_ACKNOWLEDGED` — état par défaut, pas une action à part entière.

---

### 4.10 Rôle et Permission (`Role` / `Permission`) — **[SK simplifié — RBAC à 3 rôles]**

**Définition métier** : le `Role` regroupe un ensemble de `Permission` ; la `Permission` autorise une action précise.

**Rôles retenus en SKELETON** : `USER`, `COORDINATOR`, `ADMINISTRATOR`.

**Permissions identifiées à ce stade** (liste ouverte, à compléter au fil du développement) :
- `MESSAGE_SEND`, `MESSAGE_READ`
- `FILE_UPLOAD`, `FILE_DOWNLOAD`
- `ALERT_EMIT`
- `USER_REVOKE`
- `AUDIT_READ`

**Invariant** : une action métier n'est jamais accordée "parce que c'est un `ADMINISTRATOR`" en dur dans le code — elle est toujours accordée parce que le `Role` de l'`User` porte la `Permission` correspondante.

---

### 4.11 Événement d'audit (`AuditEvent`) — **[SK]**

**Définition métier** : la trace impartiale d'une action de sécurité ou d'administration.

**Invariants** (décision AUD-01, à ne jamais contourner) :
- Un `AuditEvent` ne contient **jamais** de `PlaintextContent` de `Message` ou d'`Attachment`.
- Un `AuditEvent` ne contient **jamais** de mot de passe, token, ou clé privée.
- Exemples d'événements légitimes : `USER_LOGIN`, `USER_LOGOUT`, `TOKEN_REVOKED`, `FILE_UPLOADED`, `PERMISSION_CHANGED`.

---

## 5. CYCLES DE VIE

### 5.1 Session (`Session`)

```
Created (login réussi)
   │
   ▼
Active ──────► Renewed (refresh) ──┐
   │                                │
   │◄───────────────────────────────┘
   │
   ├──► Expired (durée de vie atteinte, sans refresh)
   │
   └──► Revoked (action explicite — ne redevient jamais Active)
```

### 5.2 Alerte (`Alert`)

```
Created (l'émetteur rédige et choisit la cible)
   │
   ▼
Broadcast (envoi effectif à l'AlertGroup — la liste des destinataires est figée ici)
   │
   ▼
Received (par chaque destinataire, indépendamment)
   │
   ├──► Acknowledged (par ce destinataire)
   │
   └──► NotAcknowledged (état par défaut, jusqu'à relance ou acquittement tardif)
   │
   ▼
Closed (décision de l'émetteur — optionnel, manuel)
```

### 5.3 Message (`Message`)

```
Drafted (côté Client uniquement — n'existe pas encore pour le Serveur)
   │
   ▼
Sent (transmis au Serveur, sous forme de OpaqueContent)
   │
   ▼
Delivered (remis au(x) destinataire(s))
   │
   ▼
Read (côté destinataire — information non garantie remontée au Serveur, cf. F1.6)
```

### 5.4 Pièce jointe — transfert (`Attachment.transferStatus`)

```
Pending ──► InProgress (par blocs) ──► Complete ──► Available
               │
               └──► Failed (reprise possible, best effort — cf. FILE-03)
```

---

## 6. RÈGLES MÉTIER EXPLICITES

- Seul un `User` avec une `Session` active peut envoyer un `Message`, émettre une `Alert`, ou accéder à un `Attachment`.
- Seul un `User` possédant la `Permission` `ALERT_EMIT` (portée par le `Role` `COORDINATOR` ou `ADMINISTRATOR`) peut créer une `Alert`.
- Seul un `User` possédant la `Permission` `USER_REVOKE` peut révoquer la `Session` d'un autre `User`.
- Un `User` ne peut acquitter (`Acknowledgment`) que les `Alert` dont il est effectivement destinataire (membre de l'`AlertGroup` ciblé au moment de la diffusion).
- Un `Message` n'est visible en clair (`PlaintextContent`) que par les participants de sa Conversation — appliqué côté Client par construction, pas par un contrôle d'accès côté Serveur sur du contenu qu'il ne peut de toute façon pas lire.
- Un `AuditEvent` est généré pour toute émission ou clôture d'`Alert`, toute révocation de `Session`, tout upload/download d'`Attachment` réussi.

---

## 7. FRONTIÈRES DU DOMAINE — CE QUI N'EST *PAS* ICI

Volontairement absent de ce document, car appartenant à `Architecture.md` ou aux spécifications techniques dédiées :

- le protocole de transport (HTTP, WebSocket, gRPC)
- le format de sérialisation des messages sur le réseau
- l'algorithme de chiffrement précis et la gestion des clés (→ `Crypto.md`, référencé par SEC-02)
- la structure des tables PostgreSQL
- tout élément d'interface graphique Qt
- l'organisation des conteneurs Docker
- la forme exacte des classes C++ — sauf la directive explicite du §4.4, seule dérogation volontaire de ce document

Un test rapide pour vérifier qu'une phrase appartient bien à ce document : *peut-on la comprendre et la discuter avec quelqu'un qui ne connaît ni gRPC ni Qt ?* Si non, elle est mal placée ici.

---

## 8. POINTS OUVERTS — À TRANCHER EN ÉQUIPE AVANT L'ARCHITECTURE

| # | Question | Statut | Impact si non tranché |
|---|---|---|---|
| 1 | Le champ `OpaqueContent` d'un `Message` inclut-il les métadonnées de l'`Attachment` (nom de fichier) ou seulement le contenu binaire ? | **Ouvert** | Un nom de fichier en clair peut fuiter de l'information sensible (ex. "Rapport_Evacuation_Kaboul.pdf") — à trancher avant F2.1/F2.2. |
| 2 | Que devient un `AlertGroup` dont tous les membres ont quitté SecureCloud (compte désactivé) ? | **Ouvert** | Impacte l'invariant §4.8. |
| 3 | Un `ADMINISTRATOR` hérite-t-il automatiquement des `Permission` d'un `COORDINATOR`, ou les rôles sont-ils strictement cloisonnés ? | **Ouvert** | Impacte directement la matrice RBAC (AUTH-03). |
| 4 | La relance après non-acquittement (UC-11, "alarme au bout d'un temps donné") est-elle une règle métier avec délai explicite, ou une simple notification technique ? | **Ouvert** | Actuellement absente du modèle si c'est une règle métier. |
| ~~5~~ | ~~Que signifie `AWAY` ?~~ | **Résolu** — `AWAY` reporté en RM V1, cf. §4.1. Seules `ONLINE`/`OFFLINE` en SKELETON. | — |

---

## 9. TRAÇABILITÉ

| Entité (nom canonique) | ADR(s) associé(s) | Statut |
|---|---|---|
| `User`, `Session` | AUTH-01, AUTH-02, AUTH-03 | SK |
| `PresenceStatus` (`ONLINE`/`OFFLINE`) | — | SK |
| `PresenceStatus` (`AWAY`) | MSG-02 | RM |
| `DirectConversation` | — | SK |
| `GroupConversation` | MSG-04 | RM (comportement) / **SK (interface anticipée, cf. §4.4)** |
| `Message` | SEC-02, SEC-03, MSG-01, MSG-03 | SK |
| `Attachment` | FILE-01, FILE-02, FILE-03, SEC-02 | SK |
| `AlertGroup`, `Alert`, `Acknowledgment` | MSG-05 | SK |
| `Role`, `Permission` | AUTH-03 | SK simplifié |
| `AuditEvent` | AUD-01 | SK |

---

**Prochaine étape proposée** : une fois les 4 points ouverts restants du §8 tranchés, ce document devient la base directe d'`Architecture.md` — chaque entité ci-dessus (avec son nom canonique anglais, déjà stable) se traduira en un contrat de données porté par un ou plusieurs microservices, sans qu'aucune nouvelle notion métier n'apparaisse à ce moment-là.




