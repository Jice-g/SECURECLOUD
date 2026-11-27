# ADR-UC1 — Authentification côté Client (UC-01 « Se connecter »)

**Projet** SECURECLOUD[SKELETON] · **Composant** Client Qt · **Version 3** · **Date** 26/11/2026
**Documents liés** : `UC1_Connexion.md` (guide de conception), `UC1_UML.md` (diagrammes), `Contrat_auth_Client-Gateway-Service.md` (**v0.1**), `Règles de conception.md` (v2), `Prompt_Implement_S0_S1a_S3_S4.md`.

> Ce document se lit seul. Il remplace la version 2 (09/11).
> **Ce qui change depuis la v2 :** le port devient **asynchrone** (signaux Qt, plus de thread de travail) et la couche **Application peut utiliser Qt**. Les décisions UC1-04 et la clause « Application sans Qt » de UC1-02 sont remplacées par **UC1-16** et **UC1-17**. Nouvelles décisions : UC1-18, UC1-19.

---

## 1. Contexte et périmètre

L'utilisateur saisit identifiant et mot de passe. Le Client les envoie en HTTPS à l'API Gateway (`POST /auth/login`). En cas de succès, il ouvre une session **en mémoire** et affiche la fenêtre principale ; en cas d'échec, il affiche un message qui ne révèle pas si l'identifiant existe.

| Dans UC1 | Hors UC1 (Roadmap) |
|---|---|
| Login nominal et ses 7 familles d'échec | MFA (`mfa_code` réservé, envoyé à `null`) |
| Session en mémoire (`SessionManager`) | Stockage durable des jetons (trousseau système) |
| Écran de connexion | Refresh automatique, révocation serveur |
| Déconnexion **locale** | Récupération d'identifiant ou de mot de passe |
| E2E-LOGIN-01 à 05 | Écran principal, messagerie |

## 2. Décisions

Statut : **décidé** sauf mention « à confirmer ».

| ID | Décision | Raison |
|---|---|---|
| **UC1-01** | Le **contrat v0.1** fait foi : JSON en **`snake_case`** (`access_token`, `refresh_token`, `token_type`, `expires_in`, bloc `user`). Rôles sur le fil et dans `UserRole` : **`ADMIN`, `COORD`, `STD_USER`**. Un rôle inconnu est une erreur (`INVALID_RESPONSE`), jamais un rôle par défaut. | Un seul contrat pour Client, Gateway et auth-service. `snake_case` est la convention des jetons OAuth 2.0. |
| **UC1-02** | Le port s'appelle **`ISessionGateway`** (défini dans Application, implémenté dans Infrastructure). Sa méthode est `login(const LoginRequestDTO&)`. `refresh` et `revoke` viendront dans le même port. | La session englobe authentification, renouvellement et état en ligne. |
| **UC1-03** | **Pas de `Result<T>`** : `shared/` reste vide. Un échec attendu (mauvais mot de passe) est une **valeur** (`AuthResult`, `LoginOutcome`). Les **exceptions** sont réservées aux violations d'invariant (`std::invalid_argument`) et aux pannes techniques (`RepositoryError`). | Un mauvais mot de passe est un résultat normal. |
| ~~UC1-04~~ | ~~Use case synchrone + thread de travail + mutex.~~ **Remplacée par UC1-16.** | — |
| **UC1-05** | **MFA hors v1** : le Client envoie `mfa_code: null`. | Périmètre. |
| **UC1-06** | Le use case s'appelle **`LoginUseCase`**. | Nom court. |
| **UC1-07** | **Session = état applicatif** (`SessionManager`), pas une entité du domaine. `SessionManager::open` met l'utilisateur `ONLINE` (BR-20). | Une session n'a pas de règle métier propre en v1. |
| **UC1-08** | **Jetons en RAM seulement**, jamais journalisés ni écrits sur disque. Le `refresh_token` est **conservé en RAM**, non utilisé en v1. | Le plus simple ; pas de retouche du mapper quand le refresh arrivera. |
| **UC1-09** | **Pas d'`IClock`.** « Maintenant » est un paramètre : `AuthJsonMapper::parseLoginResponse(status, retryAfter, body, now)`. La gateway fournit `TimeStamp::now()` ; les tests passent un instant fixe. `expiresAt = now + expires_in` (horloge du Client). | Mapper déterministe, sans nouvelle abstraction. |
| **UC1-10** | **Codes d'erreur** `AuthError` : `INVALID_INPUT`, `INVALID_CREDENTIALS`, `ACCOUNT_DISABLED`, `RATE_LIMITED`, `SERVER_UNREACHABLE`, `SERVICE_UNAVAILABLE`, `INVALID_RESPONSE`. Table statut → code : `UC1_Connexion.md` §7.3. « Identifiant inconnu » et « mot de passe faux » : **même code, même message**. | Anti-énumération de comptes. |
| **UC1-11** | **Validation locale** avant tout appel réseau : login via `UserLogin` ; mot de passe de 1 à 256 **octets** UTF-8, **jamais modifié**. Échec : `INVALID_INPUT`, aucun appel. | Pas d'appel inutile ; un mot de passe peut contenir des espaces. |
| **UC1-12** | **Pas d'effacement mémoire du mot de passe** (`SecureWipe`). La limite (copies internes de `std::string`) est documentée, pas masquée. | Une garantie qu'on ne peut pas tenir est pire que pas de garantie. |
| **UC1-13** | **Types d'échange** : `LoginRequestDTO`, `AuthSession`, `AuthResult` (fabriques `success`/`failure`, constructeur privé), `LoginOutcome` + `LoginResultDTO` (**jamais de jeton vers l'UI**). | Les secrets ne quittent pas l'Application. |
| **UC1-14** | **Sécurité réseau** : TLS ≥ 1.2, certificat **et** nom vérifiés, **jamais** `ignoreSslErrors()` (en dev on **ajoute** la CA maison) ; **aucune redirection suivie** ; pas de nouvel essai automatique ; **délai total de 20 s** (un seul minuteur) ; mot de passe uniquement dans le corps ; le Client ne décode jamais le JWT. | Contrat §2–§3. |
| **UC1-15** | **Conventions** : code, noms et commentaires en **anglais**, documentation en **français**. Classes `PascalCase`, variables `camelCase`, membres `m_xxx`, énumérations `MAJUSCULES`. Le **domaine existant garde ses méthodes en PascalCase** (`Id()`, `Online()`). JSON `snake_case`, converti à **un seul endroit** (le mapper). Tests avec le harnais maison. | Cohérence avec le dépôt. |
| **UC1-16** *(nouveau)* | **Port asynchrone par signaux Qt, sans thread de travail.** `ISessionGateway::login()` rend la main tout de suite ; la réponse arrive par le signal `loginFinished(AuthResult)`. `LoginUseCase::execute()` fait de même (signal `finished(LoginOutcome)`) et refuse un second login tant que le premier court. Tout s'exécute dans le thread UI. Connexions **directes** (même thread). | Option la plus « Qt » : `QNetworkAccessManager` est déjà asynchrone. Supprime thread, boucle locale, `QFutureWatcher` et verrou. Coût accepté : modification du port, du use case, du fake et des tests application. |
| **UC1-17** *(nouveau)* | **La couche Application peut utiliser Qt Core** (`QObject`, signaux/slots). **Le Domaine reste sans Qt** (DOM-01) — *à confirmer*. Pas de Qt Widgets ni Gui dans Application. Le `std::mutex` de `SessionManager` devient inutile : il **reste** pour l'instant, retrait en Roadmap. | Les signaux sont le moyen naturel d'exprimer l'asynchrone. Amende UC1-02. |
| **UC1-18** *(nouveau)* | **JSON en `QJsonDocument`, dans l'infrastructure.** `AuthJsonMapper` : classe sans état, fonctions `static` pures (aucun réseau, aucun `QObject`). **Lecture stricte** : tout couple statut / `error.code` non prévu → `INVALID_RESPONSE`. **Exception actée dans le contrat v0.1** (*à confirmer*) : `500, 502, 503, 504` → `SERVICE_UNAVAILABLE` sur le seul statut, corps ignoré (page d'erreur d'un proxy) ; les autres `5xx` → `INVALID_RESPONSE`. Le mapper vérifie explicitement chaque champ, puis attrape **uniquement** `std::invalid_argument` autour de la construction des objets du domaine. | Une réponse serveur étrange est un cas attendu, pas un crash. Le domaine reste propriétaire de ses règles. |
| **UC1-19** *(nouveau)* | **Configuration** par variables d'environnement `SC_GATEWAY_URL` (défaut `https://localhost:8443/auth/login`) et `SC_DEV_CA` (défaut `dev-ca.pem`). **Toute URL non-`https` est refusée** ; `main` affiche une erreur claire et quitte. Fichier de configuration : Roadmap. | Simple ; refuse de parler en clair. |

## 3. Découpage

| Couche | Éléments | Qt ? |
|---|---|---|
| domain | **inchangé** (`UserLogin`, `UserId`, `UserRole`, `User`, `TimeStamp`) | non |
| application | `AuthError`, `LoginRequestDTO`, `AuthSession`, `AuthResult`, `ISessionGateway` (**QObject**), `SessionManager`, `LoginResultDTO`, `LoginOutcome`, `LoginUseCase` (**QObject**) | Qt Core |
| infrastructure | `AuthJsonMapper`, `GatewayConfig`, `HttpsSessionGateway` (QObject), `DemoSessionGateway` (démo) | Qt Core + Network |
| ui | `LoginController` (sans thread), `UserLoginWindow`, `ClientChatWindow` | Qt Widgets |
| tests | fake gateway, tests application (existants, adaptés), tests mapper, faux serveur HTTPS | — |

Dépendances : UI → Application → Domaine ; Infrastructure → Application (elle implémente le port) et Domaine ; seul `main.cpp` connaît toutes les couches.

```cpp
class ISessionGateway : public QObject {            // the PORT
    Q_OBJECT
public:
    using QObject::QObject;
    virtual void login(const LoginRequestDTO& request) = 0;      // returns immediately
signals:
    void loginFinished(const application::AuthResult& result);   // exactly once per login()
};

class LoginUseCase final : public QObject {
    Q_OBJECT
public:
    LoginUseCase(ISessionGateway& gateway, SessionManager& session, QObject* parent = nullptr);
    void execute(const std::string& rawLogin, const std::string& password);   // asynchronous
    bool isBusy() const noexcept;
signals:
    void finished(const application::LoginOutcome& outcome);    // exactly once per accepted execute()
};
```

## 4. Déroulement

1. `UserLoginWindow` vérifie que les champs ne sont pas vides, passe en état « occupé », puis appelle `LoginController::submit`.
2. `LoginController` appelle `LoginUseCase::execute`. Si un login est déjà en cours, l'appel est ignoré.
3. Validation locale (UC1-11). Échec : `finished(INVALID_INPUT)`, **aucun appel réseau**.
4. `ISessionGateway::login` : `HttpsSessionGateway` envoie le `POST` (JSON construit par `AuthJsonMapper`) et rend la main. Minuteur de 20 s lancé.
5. La réponse arrive (signal du `QNetworkReply`). Aucun statut HTTP, ou délai dépassé : `SERVER_UNREACHABLE`. Sinon `AuthJsonMapper::parseLoginResponse(...)`. La gateway émet `loginFinished`.
6. `LoginUseCase` : succès → `SessionManager::open` puis `finished(LoginOutcome{login, rôle})` ; échec → `finished(LoginOutcome{erreur, retryAfter})` (session déjà ouverte non touchée).
7. `LoginController` émet `loginSucceeded` ou `loginFailed` ; la fenêtre ouvre la fenêtre principale ou affiche le message (champ mot de passe vidé).

## 5. Plan de travail

Deux jours. **Implémentation d'abord, tests ensuite** (choix de l'équipe).

| Étape | Contenu | Terminé quand |
|---|---|---|
| **S0** | CMake : Qt Core/Network en base, `sc_infrastructure`, `SC_DEMO_GATEWAY` OFF par défaut | build propre, 0 warning |
| **S3a** | Application asynchrone : port, use case, **fake et tests application existants adaptés** | tests application verts |
| **S1a** | `AuthJsonMapper` (code seul) | compile, relu |
| **S3b** | `GatewayConfig` + `HttpsSessionGateway` | compile, relu |
| **S4** | `LoginController` sans thread, `DemoSessionGateway` asynchrone, `main.cpp` | login manuel réussi (démo, puis vraie Gateway si disponible) |
| *Point de contrôle* | documentation de l'implémentation | — |
| **S1b** | Tests du mapper (une ligne de test par ligne de la table §7.3 du guide, `now` fixe) | tests verts |
| **S2** | Faux serveur HTTPS (`tests/tools/`) + tests réseau : 200, 401, 403, 429, 503, 3xx **non suivie**, serveur éteint, certificat inconnu refusé, délai dépassé | tests verts |
| **S5** | E2E contre la vraie pile | E2E-LOGIN-01 à 05 (ou faux serveur, en le disant) |

## 6. Limites assumées et points ouverts

- L'effacement du mot de passe n'est pas garanti (UC1-12).
- Le délai de connexion séparé (10 s du contrat) n'existe pas dans Qt : le délai total de 20 s le couvre.
- Les jetons disparaissent à la fermeture : reconnexion à chaque lancement (UC1-08).
- Le code Qt n'a **pas été compilé** par Claude (Qt absent de son environnement) : compilation et essais chez l'équipe.
- **D3** (langage du faux serveur de test : Python ou `QSslServer`) : à décider au début de la phase tests.
- À confirmer : Domaine sans Qt (UC1-17) ; exception 5xx (UC1-18).
- À obtenir de l'équipe Gateway : Q1 (URL), Q3 (comptes de test et mots de passe), Q5 (CA de dev), Q9 (`GET /health`).

## 7. Mises à jour du code et de la documentation

**Code** : le commentaire du `CMakeLists.txt` « NEVER link Qt here » est obsolète (UC1-17). L'étape « Domaine » du 08/11 (`AccessToken`, `Session`, `SessionService`) reste **inutilisée** : retrait ou conservation en Roadmap à décider en revue.
**Documentation** : `Login_features.md` (§5, §8 : plus de `Result`, `IAuthGateway`, `LoginUserUseCase`) · README et Roadmap §1.4 et §1.8 (`IAuthGateway` → `ISessionGateway`) · Domaine v0.2 §3 et §4.8 (trois niveaux de nommage des rôles) · Roadmap : retrait du mutex de `SessionManager`, `refresh`/`revoke`, MFA, stockage sécurisé des jetons, `Result<T,E>` si les erreurs attendues se multiplient.
