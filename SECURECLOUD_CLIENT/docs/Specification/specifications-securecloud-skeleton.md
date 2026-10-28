# CAHIER DES SPÉCIFICATIONS : PROJET SECURECLOUD
**SECURECLOUD[SKELETON]** : Le prototype 'Ecole'
> Plateforme de Communications Professionnelles Sécurisées pour Médecins Sans Frontières (MSF)
> Projet d'école Bachelor IT "Développeur logiciel / DevOps" - B3

---

## LÉGENDE DES DÉCISIONS SKELETON

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
Le cahier des charges présente une vision produit maximaliste (« absolument inviolable », « le serveur ne voit JAMAIS le contenu ») destinée à un cas d'usage MSF (45 000 volontaires, 70 pays, zones de conflit).

> ➜ **Décision SKELETON [DS]** — *(ADR SEC-01)* : Le texte « inviolable » / « impossibilité d'interception » est conservé tel quel dans la **Vision produit** (marketing, contexte narratif) mais **n'est jamais utilisé comme exigence technique testable**. Il est remplacé dans les spécifications ci-dessous par des propriétés vérifiables (modèle de menace explicite).

#### 1.4 Analyse Concurrentielle Référentielle
Tableau conservé sans modification — il relève du positionnement produit, pas d'une spécification technique du prototype.

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
        └── audit-service  (appelé de façon asynchrone, hors chemin critique)
               │
               ▼
        PostgreSQL (instance unique — pas de cluster en SKELETON)
```

> ➜ **Décision SKELETON [SK]** — *(ADR ARC-03)* : L'API Gateway est un **composant infrastructurel distinct** des 5 microservices métier, pas un microservice supplémentaire. Documentation à formuler ainsi : « 5 microservices métier + 1 API Gateway + 1 infrastructure de données ».

> ➜ **Décision SKELETON [DS]** — *(ADR ARC-06)* : Le **deploy-service**, tel que décrit dans le cahier des charges (microservice dédié au déploiement), n'est pas implémenté comme service applicatif séparé pour SKELETON. Son rôle est assuré directement par le pipeline CI/CD (GitHub Actions/GitLab CI) et par Docker Compose. → Roadmap V1 si un vrai microservice d'orchestration est jugé pédagogiquement pertinent.

### 2.2 Communication Inter-Services (IPC)

*   **Solution retenue** : gRPC — plus rapide qu'une API REST classique pour les appels interservices.

> ➜ **Décision SKELETON [SK]** — *(ADR ARC-01, décision actée le 16/09/2026)* : Client ↔ Gateway en **HTTP/HTTPS/WebSocket** (autorisé, nécessaire pour un client externe). Gateway ↔ microservices exclusivement en **gRPC** (pas de HTTP interservices, pas de REST consommé directement par les microservices). Le transport bas niveau du gRPC (TCP loopback vs Unix Domain Socket) est un détail d'implémentation à figer en semaine 3, non un point bloquant pour démarrer.

*   **Le C++ natif pour le Back-end**

> ➜ **Décision SKELETON [SK]** — *(ADR CPP-01)* : C++ pour les 5 microservices métier et la Gateway. Les outils périphériques (scripts CI/CD, éventuels outils de test/monitoring) ne sont **pas** contraints au C++.

*   **Le framework Qt pour l'IHM**

> ➜ **Décision SKELETON [SK]** : Qt 6 natif, cible principale de développement/démo Windows + Linux. macOS = *best effort*, non garanti faute de machine de test dans l'équipe (à documenter comme limite, pas comme échec).

---

## 3. SPÉCIFICATIONS APPLICATIVES NUMÉROTÉES

### MODULE 1 : MESSAGERIE INSTANTANÉE SÉCURISÉE
  
  

*   **Spécification F1.1 (Conversations privées et collectives)** : 1-to-1 et groupes jusqu'à 100 participants.

> ➜ **Décision SKELETON [SK / RM]** — *(ADR MSG-04)* : Conversations **1-to-1 = SK**, priorité de développement et de démonstration. **Groupes de discussion = RM (V1)** : la complexité de routage et de permissions associée n'est pas justifiée pour une démo de 6 semaines — *à valider collectivement*, cf. matrice UC → composants.
* Ce n'est pas la modélisation qui est complexe — c'est ce qu'elle déclenche en cascade techniquement. Concrètement :
* Fan-out de distribution : un message 1-to-1 a un seul destinataire à router. Un message de groupe doit être dupliqué/distribué vers N destinataires, dont certains hors ligne — il faut une file d'attente par destinataire, pas juste "un message, une livraison".
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
