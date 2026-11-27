# Règles de conception (v2)

> SECURECLOUD[SKELETON] · Client Qt · **v2 du 26/11/2026** · remplace la v1 du 30/10 (`Règles de conception.md`, conservée telle quelle à la racine du projet).
> Ces règles évitent les débats au milieu du code. Une règle = une ligne + sa raison. Les décisions détaillées sont dans `ADR_UC1_Login_Client.md` (v3).
> **Amendé** = règle de la v1 modifiée. **Nouveau** = ajouté. Sinon : inchangé.

---

## Domaine (`src/domain/`)

| ID | Règle | Raison |
|---|---|---|
| **DOM-01** | Le domaine n'utilise **ni Qt ni `QString`**. Il utilise `std::string` en UTF-8 ; la conversion se fait à la frontière UI. | Domaine testable partout, sans framework. *Confirmé le 26/11 malgré l'ouverture d'Application à Qt (APP-01).* |
| **DOM-02** | Les repositories ne font que de la **persistance locale** (SQLite). Tout ce qui part vers le serveur passe par un **port** de la couche Application (`ITransport`, `ISessionGateway`…). | Séparer stockage local et réseau. |
| **DOM-03** *(amendé)* | Les repositories sont **synchrones** (SQLite local est rapide). L'**asynchronisme** relève d'Application et d'Infrastructure, et s'exprime par des **signaux Qt** (APP-02), pas par des threads de travail. | Domaine sans Qt. |
| **DOM-04** | Les services du domaine **n'appellent pas les repositories** : purs, sans état, données en paramètres. Les use cases orchestrent : lire, appeler le service, sauvegarder. | Services triviaux à tester. |
| **DOM-05** | Le temps et les identifiants sont **injectés** : `TimeStamp now` est un paramètre, les Id sont générés par un port applicatif (`IIdGenerator`) ; pas d'horloge système dans le domaine. L'Id d'un `Message` est généré côté client (dédoublonnage, file hors-ligne). *Pour UC1 : pas d'`IClock`, `now` est un paramètre du mapper (UC1-09).* | Déterminisme. |
| **DOM-06** | Une règle qui touche **une seule entité** devient une méthode de cette entité ; une règle qui en touche **deux ou plus** devient un service. | Ex. `Alert::acknowledge` / `AlertService::create`. |
| **DOM-07** *(amendé)* | **Les échecs attendus sont des valeurs** (`AuthResult`, `LoginOutcome`), **pas de `Result<T>`** tant qu'on n'en a pas besoin (`shared/` reste vide). Les **exceptions** sont réservées aux violations d'invariant (`std::invalid_argument`) et aux pannes techniques (`RepositoryError`). Entités copiables et déplaçables ; `std::optional<T>` pour « introuvable ». | UC1-03. Un mauvais mot de passe n'est pas exceptionnel. |

## Application (`src/application/`) — *nouveau*

| ID | Règle | Raison |
|---|---|---|
| **APP-01** | La couche Application **peut utiliser Qt Core** (`QObject`, signaux/slots, `QTimer`). **Pas** de Qt Widgets ni Gui. | UC1-17 : les signaux expriment naturellement l'asynchrone. |
| **APP-02** | Un **port asynchrone** est un `QObject` dont la méthode **retourne immédiatement** et dont le résultat arrive par **un signal émis exactement une fois par appel**. Connexions **directes**, même thread (jamais `Qt::QueuedConnection` : les types de résultat ne sont pas enregistrés). | UC1-16. |
| **APP-03** | Un implémenteur de port **peut** émettre le signal **avant** que la méthode ne retourne (fake de test). L'appelant doit donc être prêt : état « occupé » posé **avant** l'appel, connexion faite **avant** l'appel. | Tests simples et synchrones. |
| **APP-04** | Un use case asynchrone **refuse un second appel** tant que le premier court (`isBusy()`), et remet son état « libre » **avant** d'émettre son résultat. | Une seule opération à la fois ; permet de relancer depuis le slot. |
| **APP-05** | Les **secrets** (jetons, mot de passe) **ne quittent jamais** la couche Application : l'UI reçoit des DTO sans jeton ; rien n'est journalisé. | Sécurité (UC1-13). |
| **APP-06** | Un use case ne connaît que des **ports** (`ISessionGateway`…), jamais l'adaptateur concret. Il reçoit ses dépendances par le **constructeur** (références, non possédées). | Testable avec un fake. |

## Infrastructure (`src/infrastructure/`) — *nouveau*

| ID | Règle | Raison |
|---|---|---|
| **INF-01** | La traduction du format réseau (JSON, statuts HTTP) se fait dans un **mapper sans état, fonctions `static` pures** (pas de réseau, pas de `QObject`, pas d'horloge). | Testable avec des chaînes écrites en dur. |
| **INF-02** | **Lecture stricte** des réponses : champ manquant, mal typé, couple statut/code inattendu → `INVALID_RESPONSE`. Seule exception : `500/502/503/504` → `SERVICE_UNAVAILABLE` sur le seul statut. Le mapper vérifie chaque champ, puis attrape **uniquement** `std::invalid_argument` autour de la construction des objets du domaine (jamais `catch (...)`). | UC1-18. |
| **INF-03** | **TLS toujours vérifié** : version ≥ 1.2, certificat **et** nom ; **jamais** `ignoreSslErrors()` (en dev, on **ajoute** une CA). | UC1-14. |
| **INF-04** | **Aucune redirection suivie** (`ManualRedirectPolicy`) ; **pas de nouvel essai** automatique ; **un seul délai total** (20 s). | Le mot de passe ne doit pas partir ailleurs ; un login n'est pas idempotent. |
| **INF-05** | On décide **par le statut HTTP**, pas par `reply->error()`. Aucun statut du tout → `SERVER_UNREACHABLE`. | Un 401 est une réponse métier valide. |
| **INF-06** | Journalisation autorisée : statut, nom de l'erreur, durée, `X-Request-Id`. **Jamais** le corps, le mot de passe ni les jetons. | Sécurité. |
| **INF-07** | Configuration par variables d'environnement ; toute URL non-`https` est refusée. | UC1-19. |

## UI (`src/ui/`) — *nouveau*

| ID | Règle | Raison |
|---|---|---|
| **UI-01** | L'UI connaît **Application**, jamais Infrastructure. Aucune règle métier dans les fenêtres (contrôle de forme seulement). | Séparation des couches. |
| **UI-02** | Les signaux du contrôleur ne portent que des **types simples** (`QString`, `int`) : aucun type Application ni jeton ne traverse vers les widgets. | UC1-13. |
| **UI-03** | La fenêtre pose son état « occupé » **avant** d'appeler le contrôleur, et **vide le champ mot de passe** après envoi. | Compatible avec une réponse synchrone (APP-03). |
| **UI-04** | Les textes affichés sont traduisibles (`tr()`), indexés par code d'erreur. « Identifiant inconnu » et « mot de passe faux » ont **le même message**. | Anti-énumération. |

## Conventions et tests

| ID | Règle | Raison |
|---|---|---|
| **NAM-01** | Code, noms et commentaires en **anglais** ; documentation en **français**. | Consigne de l'école. |
| **NAM-02** | Classes `PascalCase`, variables `camelCase`, membres `m_xxx`, énumérations `MAJUSCULES`. Le **domaine existant garde ses méthodes en PascalCase** (`Id()`, `Online()`). | Cohérence avec l'existant. |
| **NAM-03** | JSON du contrat en **`snake_case`**, codes d'erreur en `MAJUSCULES_AVEC_TIRETS_BAS`. La conversion vers le C++ se fait **à un seul endroit** (le mapper), avec des constantes nommées. | UC1-01, UC1-15. |
| **TST-01** | Harnais de tests maison (`CHECK`, `RUN_TEST`) ; **fakes écrits à la main** ; tests en Arrange / Act / Assert ; 0 warning. | UC1-15. |
| **TST-02** | Les tests Application et mapper tournent **sans réseau** et sans Widgets. Les tests réseau utilisent un faux serveur HTTPS qui implémente **le même contrat**. | Rapidité, reproductibilité. |
| **TST-03** | Sanitizers : address + undefined d'un côté, thread de l'autre (à relancer si un thread réapparaît). | Détecter les erreurs invisibles. |
| **TST-04** | Un test réseau ne contourne **jamais** la vérification TLS ; le certificat de dev est généré par un script et la CA ajoutée à la configuration. | Tester la vraie sécurité. |
