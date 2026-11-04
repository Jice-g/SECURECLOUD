# Registre des décisions d'architecture (ADR) : niveau 1

> **Version** 1.0 · **Date** 04/11/2026 · **Statut** brouillon à relire · **Niveau** 1 (commun Client / Serveur)
> **Documents liés** : [../specifications/specifications_skeleton.md](../specifications/specifications_skeleton.md) · [../school_vision/analyse_cahier_des_charges.md](../school_vision/analyse_cahier_des_charges.md) · [correspondance_identifiants.md](correspondance_identifiants.md)

## 0. Mode d'emploi

**Rôle.** L'ADR dit **ce qu'on a décidé** ; la spécification dit **ce qu'on construit** ; l'analyse dit **pourquoi**. Chacun renvoie aux autres par identifiant, sans recopier.

**Périmètre.** Ce registre contient les décisions **transverses** (Client et Serveur). Les décisions propres au Client ont leur registre dans `client/docs/decisions/` (lot suivant).

**Identifiants.** `THÈME-NN`, stables : `SEC` sécurité, `ARC` architecture, `CPP` langage, `AUTH` authentification, `FILE` fichiers, `MSG` messagerie, `AUD` audit, `PERF` performance, `OPS` exploitation, `QT` interface, `CI` intégration continue, `TEST` tests. On les cite sous cette forme courte (« ADR SEC-02 »). Un identifiant n'est jamais réutilisé.

**Statuts.**

| Statut | Sens |
|---|---|
| **acté** | Décidé par l'équipe et noté comme tel dans les documents de travail |
| **proposé** | Recommandé, **à valider** par l'équipe (puis par l'encadrement pour les écarts au cahier) |
| **à décider** | Question ouverte, avec options et recommandation |
| **remplacé** | Remplacé par une autre décision (référence indiquée) |

**Périmètre produit** : SK, SK simplifié, RM, DS (voir `prototype_skeleton.md` §2).

**Modifier une décision.** On ne réécrit pas l'historique : on passe l'ancienne en « remplacé » et on ajoute une fiche.

> **Les statuts ci-dessous reflètent ce que disent les documents de travail. Ils sont à relire et à valider par l'équipe.**

---

## 1. Index

| ID | Décision | Périmètre | Statut |
|---|---|---|---|
| SEC-01 | « Inviolable » : vision, remplacée par un modèle de menace | SK (modèle) / DS (absolu) | proposé |
| SEC-02 | Aucune cryptographie maison ; spécification cryptographique dédiée | SK | proposé |
| SEC-03 | Le serveur ne voit jamais le **contenu** ; frontière des métadonnées | SK | acté |
| SEC-04 | Recherche exclusivement locale | SK | proposé |
| ARC-01 | REST et WebSocket à l'extérieur, gRPC à l'intérieur | SK | acté (16/09/2026) |
| ARC-02 | WebSocket Client → Gateway ; streaming gRPC derrière | SK | proposé |
| ARC-03 | La Gateway est un composant d'infrastructure distinct | SK | proposé |
| ARC-04 | PostgreSQL : instance unique | SK / RM | proposé |
| ARC-05 | Docker Compose mono-nœud ; architecture cible réplicable | SK / RM | proposé |
| ARC-06 | `deploy-service` reporté | RM | proposé |
| ARC-07 | Chemin critique : audit asynchrone, autorisation en cache | SK | proposé |
| CPP-01 | C++ pour les services et la Gateway | SK | proposé |
| AUTH-01 | Modèle de jetons : accès court, rafraîchissement, version de session | SK | acté |
| AUTH-02 | Pas d'authentification maison ; hachage standard ; MFA | SK | proposé |
| AUTH-03 | RBAC à trois rôles ; permissions portées par le rôle | SK simplifié | acté |
| AUTH-04 | 45 000 utilisateurs : objectif d'architecture, non démontré | DS (volume) / RM | proposé |
| AUTH-05 | Révocation « immédiate » redéfinie | SK | acté |
| AUTH-06 | Normalisation du login en minuscules | SK | acté (01/11/2026) |
| AUTH-07 | Résistance à la force brute par limitation de débit | SK | proposé |
| FILE-01 | Sauvegarde terrain : stockage local, file d'attente, synchronisation différée | SK simplifié | proposé |
| FILE-02 | Pas de déduplication côté serveur | DS | acté |
| FILE-03 | Protocole de transfert par blocs | SK | proposé |
| FILE-04 | Compression avant chiffrement | SK | proposé |
| FILE-05 | Pas de système de fichiers virtuel maison | SK simplifié | proposé |
| FILE-06 | Nom de fichier d'une pièce jointe | SK | à décider |
| MSG-01 | Thread = réponse à un message | SK simplifié | proposé |
| MSG-02 | Présence `ONLINE` / `OFFLINE` ; notifications par règles | SK simplifié | acté (16/09/2026) |
| MSG-03 | Priorité logique à trois niveaux | SK simplifié | proposé |
| MSG-04 | 1-to-1 en SK ; groupes de discussion en RM | SK / RM | acté |
| MSG-05 | Alertes : groupe d'alerte distinct, suivi d'acquittement | SK | acté |
| MSG-06 | Statuts de livraison et idempotence | SK | à décider |
| MSG-07 | La conversation naît au premier message | SK | proposé |
| AUD-01 | L'audit enregistre des événements, jamais du contenu | SK | acté |
| PERF-01 | 10 000 msg/s : méthode SK, valeur DS | SK / DS | proposé |
| PERF-02 | Latence : métrique et percentiles | SK | proposé |
| PERF-03 | Fichier de 500 Mo et 256 kbps : deux scénarios | SK | proposé |
| OPS-01 | « Zéro panne » supprimé ; 99,9 % objectif cible | SK reformulé | proposé |
| OPS-02 | Scalabilité : définition et démonstration | SK | à décider |
| QT-01 | Qt 6 natif ; Windows et Linux ; macOS best effort | SK / RM | proposé |
| QT-02 | Démarrage et mémoire : mesures ponctuelles | SK | proposé |
| QT-03 | Résilience environnementale : seul le réseau est simulé | SK partiel / DS | proposé |
| CI-01 | Pipeline : PR et branche principale ; pas de production à chaque commit | SK | proposé |
| CI-02 | « 5 minutes » : environnement de démonstration | SK | proposé |
| TEST-01 | « Couverture 100 % » : par cas d'usage | SK | proposé |
| TEST-02 | Audit externe : non réalisé, revue interne | DS | proposé |

---

## 2. Fiches : sécurité

### SEC-01 — « Inviolable » : vision, remplacée par un modèle de menace
**Statut** proposé · **Périmètre** SK (modèle de menace) / DS (propriété absolue)
**Contexte.** Le cahier promet une plateforme « inviolable », sans interception possible, résistante aux attaques étatiques. Ce ne sont pas des propriétés testables.
**Décision.** Ces formules restent dans la vision produit. Elles sont remplacées par un **modèle de menace** (attaquant réseau passif ou actif, serveur compromis sans les clés, vol de session, poste Client compromis, clé compromise) et des propriétés vérifiables.
**Conséquences.** Document `threat_model.md` à rédiger ; limites explicitement documentées. Aucune architecture n'est bâtie sur une affirmation impossible à démontrer.

### SEC-02 — Aucune cryptographie maison
**Statut** proposé · **Périmètre** SK
**Contexte.** « AES-256 partout » n'est pas une spécification : manquent clés, nonces, authentification, rotation, révocation.
**Décision.** Les primitives viennent d'une bibliothèque éprouvée (**libsodium**). Algorithmes, modes opératoires, gestion des clés et protocole sont définis dans une **spécification cryptographique dédiée** (`Crypto.md`) **avant** le développement du module. Le choix de l'algorithme (AES-256-GCM, qui dépend du support matériel de libsodium, ou XChaCha20-Poly1305) y est tranché.
**Conséquences.** `Crypto.md` est un prérequis du chiffrement de bout en bout, des fichiers et du stockage local.

### SEC-03 — Le serveur ne voit jamais le contenu
**Statut** acté · **Périmètre** SK
**Contexte.** Le routage exige des métadonnées ; « le serveur ne sait rien » serait faux.
**Décision.** Le **contenu** des messages et des fichiers est chiffré côté Client et jamais déchiffrable par le serveur. **Visibles du serveur** : expéditeur, conversation, destinataires, priorité, horodatage, taille de l'enveloppe, identité (login, rôle). PostgreSQL ne stocke jamais de contenu en clair.
**Conséquences.** Le domaine distingue `PlaintextContent` (Client) de `OpaqueContent` (serveur). Le nom d'un fichier joint est traité par FILE-06.

### SEC-04 — Recherche exclusivement locale
**Statut** proposé · **Périmètre** SK
**Décision.** Le serveur n'offre aucun moteur de recherche sur le contenu. La recherche porte sur les messages déchiffrés, côté Client, sur un index local (simple recherche textuelle en SKELETON).

---

## 3. Fiches : architecture

### ARC-01 — REST et WebSocket à l'extérieur, gRPC à l'intérieur
**Statut** acté (16/09/2026) · **Périmètre** SK
**Contexte.** Le cahier impose « pas de HTTP entre services » tout en faisant consommer l'API REST d'Auth par les autres services : contradiction. Il propose aussi named pipes, sockets de domaine et mémoire partagée.
**Décision.** Client ↔ Gateway : HTTPS et WebSocket sécurisé. Gateway ↔ services : **gRPC exclusivement**. La Gateway convertit REST en gRPC. Aucun service ne consomme l'API REST d'un autre. Named pipes et mémoire partagée écartés.
**Point ouvert.** Transport sous-jacent du gRPC (TCP sur le réseau Compose ou socket de domaine). Un socket de domaine entre conteneurs exige un volume partagé ; TCP sur le réseau interne est plus simple. À figer en semaine 3, sans bloquer le démarrage.

### ARC-02 — WebSocket Client → Gateway, streaming gRPC derrière
**Statut** proposé · **Périmètre** SK
**Décision.** La messagerie temps réel passe par WebSocket entre le Client et la Gateway, et par streaming gRPC entre la Gateway et le messaging-service.

### ARC-03 — La Gateway est un composant d'infrastructure
**Statut** proposé · **Périmètre** SK
**Contexte.** La Gateway figure dans l'architecture mais pas dans la liste des « 5 microservices » ni dans Compose.
**Décision.** API Gateway = composant d'infrastructure distinct des services métier. Formulation : **« 4 services métier (auth, messaging, files, audit) + 1 API Gateway + 1 infrastructure de données »**. Compose contient au minimum `gateway`, `auth-service`, `messaging-service`, `files-service`, `audit-service`, `postgresql`.

### ARC-04 — PostgreSQL : instance unique
**Statut** proposé · **Périmètre** SK / RM
**Décision.** Une seule instance dans Compose. Pour l'architecture cible, PostgreSQL est un composant pouvant être déployé en haute disponibilité ; réplication et cluster ne sont pas implémentés (RM-21).

### ARC-05 — Docker Compose mono-nœud
**Statut** proposé · **Périmètre** SK / RM
**Décision.** SKELETON utilise Docker Compose, qui n'apporte ni redondance, ni basculement, ni répartition de charge. L'architecture cible est conçue pour être réplicable ; la haute disponibilité est en RM.

### ARC-06 — `deploy-service` reporté
**Statut** proposé · **Périmètre** RM
**Contexte.** Le cahier prévoit un microservice de déploiement ; il doublonne le CI/CD. Il était noté DS dans la spécification v1 et « ?? » dans une copie : **classement corrigé en RM**.
**Décision.** Non implémenté en SKELETON ; son rôle est tenu par la pipeline CI/CD et Docker Compose. À reconsidérer en V1 si un vrai microservice d'orchestration se justifie.

### ARC-07 — Chemin critique
**Statut** proposé · **Périmètre** SK
**Contexte.** Chaque service ajouté sur le trajet d'un message ajoute de la latence et un point de panne.
**Décision.** Ne pas mettre tous les services sur le chemin critique : l'audit est **asynchrone** ; l'autorisation utilise un cache après validation du jeton. Le comportement en cas de panne de chaque dépendance (Auth, Files, Audit) est documenté et testé en intégration.

### CPP-01 — C++ pour les services et la Gateway
**Statut** proposé · **Périmètre** SK
**Décision.** Services métier et Gateway en C++. Les outils périphériques (scripts CI/CD, outils de test et de supervision) ne sont pas contraints au C++.

---

## 4. Fiches : authentification

### AUTH-01 — Modèle de jetons
**Statut** acté · **Périmètre** SK
**Contexte.** Le cahier cite « JWT custom ». Un JWT autoportant ne se révoque pas seul.
**Décision.** Bibliothèque JWT standard (jwt-cpp), aucun format maison. Modèle : **jeton d'accès court + jeton de rafraîchissement + version de session côté Auth**. Les jetons sont opaques pour le Client. Algorithme de signature, claims, expiration, émetteur, audience et rotation sont détaillés dans le contrat d'authentification.

### AUTH-02 — Pas d'authentification maison
**Statut** proposé · **Périmètre** SK
**Décision.** Pas de « bcrypt optimisé » développé en interne : bibliothèque standard (bcrypt ou argon2). MFA : TOTP si le temps le permet, sinon MFA simulé ; mécanisme documenté dans les deux cas.

### AUTH-03 — RBAC à trois rôles
**Statut** acté · **Périmètre** SK simplifié
**Décision.** Trois rôles fixes. Une autorisation découle de la **permission portée par le rôle**, jamais d'un nom de rôle écrit en dur. **Trois niveaux de nommage, pas trois conflits** : concepts métier (utilisateur, coordinateur, administrateur) dans la documentation ; valeurs d'énumération `STD_USER`, `COORD`, `ADMIN` dans le code **et sur le fil**. Permissions identifiées (liste ouverte) : `MESSAGE_SEND`, `MESSAGE_READ`, `FILE_UPLOAD`, `FILE_DOWNLOAD`, `ALERT_EMIT`, `USER_REVOKE`, `AUDIT_READ`.
**Point ouvert.** Un administrateur hérite-t-il des permissions d'un coordinateur ? À trancher avant la matrice.

### AUTH-04 — 45 000 utilisateurs
**Statut** proposé · **Périmètre** DS (volume) / RM
**Décision.** Test avec quelques centaines d'utilisateurs simulés. Les 45 000 restent un objectif d'architecture, non démontré.

### AUTH-05 — Révocation « immédiate »
**Statut** acté · **Périmètre** SK
**Décision.** « Immédiat » signifie : **aucun nouveau traitement authentifié n'est accepté après propagation de la révocation**. Mécanisme : version de session côté Auth, vérifiée par la Gateway ; le jeton d'accès court limite la fenêtre. Testé fonctionnellement ; aucun SLA de latence en SKELETON (RM-16). Une session révoquée ne redevient jamais active.

### AUTH-06 — Normalisation du login
**Statut** acté (01/11/2026) · **Périmètre** SK
**Décision.** Le login est normalisé **en minuscules**, comparé sans tenir compte de la casse. Espaces de début et de fin retirés ; espace interne refusé ; jeu de caractères limité. Le Client envoie la forme normalisée ; **le serveur renormalise et valide de son côté**, sans faire confiance au Client. Le mot de passe n'est jamais modifié ni normalisé.

### AUTH-07 — Résistance à la force brute
**Statut** proposé · **Périmètre** SK
**Décision.** Limitation de débit à la Gateway **avant** l'appel à Auth, par login et par adresse IP ; réponse `429` avec `Retry-After`. Réponse et temps de réponse identiques pour « identifiant inconnu » et « mot de passe faux ». Seuil de départ : 5 échecs en 5 minutes par login (à confirmer avec l'équipe serveur). S'ajoutent hachage lent du mot de passe et MFA.

---

## 5. Fiches : fichiers

### FILE-01 — Sauvegarde terrain sans Internet
**Statut** proposé · **Périmètre** SK simplifié
**Décision.** Client → stockage local chiffré → file d'attente → synchronisation dès le retour du réseau. Testé par coupure simulée. Support externe ou serveur local : RM-11.

### FILE-02 — Pas de déduplication côté serveur
**Statut** acté · **Périmètre** DS
**Contexte.** Deux fichiers identiques chiffrés avec des nonces différents donnent des blocs différents.
**Décision.** Hors périmètre. Retour en roadmap uniquement si un mécanisme compatible avec le chiffrement de bout en bout est spécifié (RM-09).

### FILE-03 — Protocole de transfert par blocs
**Statut** proposé · **Périmètre** SK
**Décision.** Protocole minimal : `TransferId`, `ChunkId`, `TotalChunks`, `Payload`, `IntegrityTag`. Reprise = relance du bloc en échec. Taille de bloc, ordre, stockage temporaire et suppression des fragments incomplets à préciser dans le contrat Files. À figer **conjointement** par le Client et le service Files.

### FILE-04 — Compression avant chiffrement
**Statut** proposé · **Périmètre** SK
**Décision.** Ordre : donnée → compression → chiffrement → transmission, lorsque la compression est applicable (des données déjà chiffrées ne se compressent pas). Le risque lié à la compression de données secrètes dans certains protocoles interactifs est noté.

### FILE-05 — Pas de système de fichiers virtuel maison
**Statut** proposé · **Périmètre** SK simplifié
**Décision.** Stockage chiffré fichier par fichier dans un répertoire dédié, avec un index chiffré simple. Un vrai système de fichiers virtuel est hors périmètre (RM-10).

### FILE-06 — Nom de fichier d'une pièce jointe
**Statut** à décider · **Périmètre** SK
**Contexte.** Un nom comme « Rapport_Evacuation_Kaboul.pdf » est sensible.
**Options.** (a) nom en clair pour le serveur ; (b) nom inclus dans le contenu opaque.
**Recommandation.** (b) : le serveur ne voit alors que la taille. À trancher **avant** le module fichiers.

---

## 6. Fiches : messagerie

### MSG-01 — Threads
**Statut** proposé · **Périmètre** SK simplifié
**Décision.** Un thread est la réponse simple à un message ciblé, sans arborescence. Un modèle multi-niveaux est en RM-04.

### MSG-02 — Présence et notifications
**Statut** acté (16/09/2026) · **Périmètre** SK simplifié
**Décision.** `ONLINE` et `OFFLINE`, liées à l'état de la session (pas de bascule manuelle, pas d'historique). **`AWAY` est réservé V1** (troisième valeur du même type). Notifications par règles explicites : nouveau message, alerte, ajout à un groupe d'alerte ; désactivables. « Intelligentes » non retenu.

### MSG-03 — Priorité logique
**Statut** proposé · **Périmètre** SK simplifié
**Décision.** Trois niveaux `NORMAL`, `HIGH`, `CRITICAL`, avec alerte visuelle et sonore côté Client sur `CRITICAL`. La priorité est visible du serveur (elle sert au routage). **Priorité logique ne signifie pas garantie de livraison instantanée.**

### MSG-04 — Groupes de discussion
**Statut** acté · **Périmètre** SK (1-to-1) / RM (groupes)
**Décision.** Le 1-to-1 est la priorité. Les groupes sont reportés (RM-01). Le **modèle anticipe** les groupes (base commune `Conversation`), pour qu'ils soient une extension, jamais une réécriture. Un envoi vers un groupe est **refusé avec une erreur explicite** « fonction non disponible », jamais ignoré.

### MSG-05 — Alertes et groupes d'alerte
**Statut** acté · **Périmètre** SK
**Décision.** Un **groupe d'alerte** (population cible) est distinct d'un groupe de discussion. Une alerte a son propre cycle de vie (créée, diffusée, reçue, acquittée, éventuellement clôturée). L'« accusé forcé » devient un **suivi d'acquittement** visible de l'émetteur. Les destinataires sont **figés à la diffusion**. En SKELETON, les groupes d'alerte sont préchargés (UC-12 en RM). Émettre une alerte exige la permission `ALERT_EMIT`.
**Point ouvert.** La relance après non-acquittement est-elle une règle métier avec délai (RM-06) ?

### MSG-06 — Statuts de livraison et idempotence
**Statut** à décider · **Périmètre** SK
**Contexte.** Deux lectures de `SENT` / `DELIVERED` coexistent. Lecture A : `SENT` = acquitté par le serveur, `DELIVERED` = acquitté par l'appareil du destinataire. Lecture B : `SENT` = transmis depuis le Client, `DELIVERED` = la Gateway a validé et enregistré le message pour le destinataire.
**Recommandation.** Une définition unique, **lecture A** (un accusé du serveur est un fait vérifiable côté Client) ; `DELIVERED` optionnel en SKELETON. **Idempotence** : l'identifiant du message est généré par le Client ; le serveur ignore un identifiant déjà reçu et renvoie le même accusé, ce qui rend le renvoi sans risque de doublon. Au redémarrage, un message resté `SENDING` est traité comme non livré, puis remis en file. La cause d'un échec (connexion ou serveur) est conservée.
**Conséquences.** La définition engage le contrat Client-Serveur : à figer avec l'équipe serveur avant UC-02.

### MSG-07 — La conversation naît au premier message
**Statut** proposé · **Périmètre** SK
**Décision.** Le Client génère l'identifiant de conversation. L'enveloppe du premier message porte l'identifiant, l'expéditeur et le destinataire. Le serveur **crée la conversation si elle est absente** et **refuse** si l'identifiant existe avec d'autres participants. Une contrainte d'unicité porte sur la paire canonique de participants (self-chat inclus). Cela permet l'écriture hors ligne et évite un aller-retour.
**Point ouvert.** Un identifiant dérivé de la paire triée serait préférable à un UUID, car il évite toute coordination entre deux appareils d'un même utilisateur.

---

## 7. Fiches : audit, performance, exploitation

### AUD-01 — L'audit enregistre des événements, jamais du contenu
**Statut** acté · **Périmètre** SK
**Décision.** L'audit-service enregistre des **événements de sécurité et d'administration** (`USER_LOGIN`, `USER_LOGOUT`, `TOKEN_REVOKED`, `FILE_UPLOADED`, `PERMISSION_CHANGED`…). Jamais le contenu déchiffré d'un message ou d'un fichier, ni mot de passe, jeton ou clé privée. Un événement est généré pour toute émission ou clôture d'alerte, toute révocation, tout transfert de fichier réussi. Asynchrone, hors chemin critique. Chaîne d'inaltérabilité en RM-18.

### PERF-01 — 10 000 messages par seconde
**Statut** proposé · **Périmètre** SK (méthode) / DS (valeur)
**Décision.** Un scénario reproductible est défini : nombre de clients, taille moyenne des messages, durée, latence P50/P95/P99, taux d'erreur, CPU et RAM. On publie : « objectif cible 10 000 msg/s ; valeur mesurée : X msg/s ». Une moyenne seule masque les pires cas : on utilise les percentiles.

### PERF-02 — Latence
**Statut** proposé · **Périmètre** SK
**Décision.** « Temps de réponse < 100 ms » = latence **Client A → réception Client B**, mesurée en **P95/P99** dans les conditions nominales du benchmark. Pour la validation de jeton, la latence de `/auth/validate` est mesurée de la même façon.

### PERF-03 — Fichier de 500 Mo et liaison à 256 kbps
**Statut** proposé · **Périmètre** SK
**Contexte.** 500 Mo à 256 kbps prennent plus de 4 heures ; tenir 3 minutes exige au moins 22 Mbit/s.
**Décision.** Deux scénarios, **jamais présentés comme simultanés** : (1) réseau de démonstration normal, 500 Mo en moins de 3 minutes ; (2) lien dégradé de 256 kbps simulé, avec découpage, reprise, file d'attente, sans contrainte de temps.

### OPS-01 — « Zéro panne » et 99,9 %
**Statut** proposé · **Périmètre** SK reformulé
**Décision.** « Zéro tolérance à la panne » est supprimé des exigences testables. À la place : absence de point unique de défaillance évident dans l'architecture applicative et reprise contrôlée après le crash d'un service (redémarrage automatique du conteneur). 99,9 % est un **objectif d'architecture cible**, non mesuré avec le prototype.

### OPS-02 — Scalabilité
**Statut** à décider · **Périmètre** SK
**Contexte.** La scalabilité est un enjeu prioritaire du projet mais n'est pas définie.
**Proposition.** Services conçus **sans état local** (l'état est dans PostgreSQL) ; capacité de lancer plusieurs instances d'au moins un service sans état (par exemple `auth-service`) derrière la Gateway, démontrée par `docker compose up --scale`. La messagerie temps réel (WebSocket) demande un traitement particulier et n'est pas retenue pour cette démonstration.

---

## 8. Fiches : interface, CI/CD, tests

### QT-01 — Qt 6 natif
**Statut** proposé · **Périmètre** SK / RM
**Décision.** Qt 6 natif. Build et tests validés sur **Windows et Linux**. macOS en *best effort*, faute de machine de test : limite documentée, pas non-conformité (RM-26). Version et chaîne d'outils : documentation du Client.

### QT-02 — Démarrage et mémoire
**Statut** proposé · **Périmètre** SK
**Décision.** Démarrage < 2 s et mémoire < 50 Mo mesurés ponctuellement, sans benchmark automatisé en CI (RM-25).

### QT-03 — Résilience environnementale
**Statut** proposé · **Périmètre** SK partiel / DS
**Décision.** Seul le **réseau dégradé** (256 kbps simulé) est testé. Chaleur, poussière et matériel vieillissant sont non démontrables et documentés comme tels.

### CI-01 — Pipeline
**Statut** proposé · **Périmètre** SK
**Décision.** Pipeline sur GitHub : **pull request** = build, tests unitaires, tests d'intégration, package ; **branche principale** = idem + déploiement sur environnement de démonstration + contrôle de santé. Étapes de sécurité : analyse statique, scan des images, secrets hors dépôt, retour arrière documenté. Pas de déploiement automatique en « production » à chaque commit : à éviter pour une plateforme de sécurité. Signature d'artefacts et approbation : RM-24.

### CI-02 — « 5 minutes »
**Statut** proposé · **Périmètre** SK
**Décision.** Push sur la branche de déploiement → pipeline complète → environnement de **démonstration** opérationnel en ≤ 5 minutes. Durée mesurée par étape.

### TEST-01 — « Couverture 100 % »
**Statut** proposé · **Périmètre** SK
**Décision.** 100 % des **cas d'usage retenus pour SKELETON** sont couverts par au moins un test d'intégration. La couverture de lignes est suivie à titre indicatif. La campagne de tests est présentée à l'encadrement.

### TEST-02 — Audit externe
**Statut** proposé · **Périmètre** DS
**Décision.** Aucun audit externe : hors capacité et hors budget. Remplacé par une revue cryptographique interne documentée (primitives, analyse des menaces, tests, analyse statique). Mention explicite dans les livrables : « audit externe professionnel non réalisé dans le cadre du projet ».

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 1.0 | 04/11/2026 | Registre repris « au propre ». Identifiants thématiques conservés ; fiches ajoutées : SEC-04, ARC-07, AUTH-05, AUTH-06, AUTH-07, FILE-04, FILE-05, FILE-06, MSG-06, MSG-07, OPS-02. `ARC-06` passe en RM |
