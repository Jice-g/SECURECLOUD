# CONTRAT CLIENT ↔ API GATEWAY : AUTHENTIFICATION (v0)

> SECURECLOUD[SKELETON] · proposé le lundi 2 novembre 2026 · **à l'équipe SECURECLOUD** pour *Gateway / auth-service*  
> Périmètre : `POST /auth/login` (détaillé, E2E pour 6/11).
> `refresh` et `revoke` : réservés, esquissés au §10.

<br>

**Légende :**  
**MUST** = obligatoire pour le E2E · **SHOULD** = recommandé · **[Q]** = question ouverte.

<br>

---

## 1. Vue d'ensemble : qui parle à qui ?

```
Client Qt ── HTTPS (TCP 443/8443, TLS) ──► API Gateway ── gRPC (réseau interne) ──► auth-service ──► PostgreSQL
   │                                          │
   │  JSON                                    │  protobuf
   └─ ne voit JAMAIS auth-service             └─ route publique : pas de JWT à valider sur /auth/login
```

**Trois règles pour le Login :**
- Le Client ne connaît **que** la Gateway. HTTP/JSON de bout en bout côté Client, gRPC derrière pour relayer aux micro-services.
- `POST /auth/login` est une **route publique** : la Gateway ne valide aucun JWT dessus (l'utilisateur n'en a pas encore).
- Le Client ne parse jamais le JWT : `access_token` et `refresh_token` sont des **chaînes opaques** pour lui.
<br>

---

## 2. Logique réseau d'un login, couche par couche

Ce que fait réellement le Client quand l'utilisateur clique « Se connecter » :

| # | Couche | Étape | Détail | Échec possible → `AuthError` |
|---|---|---|---|---|
| 1 | Application (DNS) | Résolution du nom de la Gateway | UDP/53 (ou cache OS) | Nom introuvable → `SERVER_UNREACHABLE` |
| 2 | Transport (TCP) | Poignée de main à 3 temps | `SYN` → `SYN-ACK` → `ACK` vers `gateway:8443` (dev) / `:443` (prod) | Refus (`RST`), pas de réponse (timeout) → `SERVER_UNREACHABLE` |
| 3 | Session (TLS) | Poignée de main TLS | `ClientHello` (avec **SNI** = nom de la Gateway) → `ServerHello` + certificat → vérification de la chaîne **et du nom** → `Finished` | Certificat inconnu / expiré / mauvais nom, version TLS refusée → `SERVER_UNREACHABLE` (**jamais** de contournement) |
| 4 | Application (HTTP) | Envoi de la requête | `POST /auth/login`, en-têtes + corps JSON, dans le canal chiffré | Connexion coupée pendant l'envoi → `SERVER_UNREACHABLE` |
| 5 | Gateway | Traitement | Limitation de débit → validation du JSON → appel gRPC `auth-service` (délai max 5 s) | voir §6 |
| 6 | Application (HTTP) | Réponse | Code de statut + en-têtes + corps JSON | Pas de réponse avant le délai global → `SERVER_UNREACHABLE` ; corps illisible → `INVALID_RESPONSE` |
| 7 | Transport | Fin | Connexion gardée (keep-alive) ou fermée ; le Client **ne réutilise pas** une connexion pour autre chose que la Gateway | — |

**Ce qu'un observateur du réseau voit** (le mot de passe n'en fait pas partie) : l'adresse IP de la Gateway, le nom demandé (DNS et SNI), la taille et le rythme des paquets. **Ce qu'il ne voit pas** : le chemin `/auth/login`, les en-têtes, le login, le mot de passe, les tokens, la réponse. D'où la règle : **le mot de passe voyage uniquement dans le corps, jamais dans l'URL** (les URL finissent dans les journaux).

### Délais (connexion satellite incluse)

Un satellite géostationnaire ajoute environ 600 ms d'aller-retour (RTT). Un login complet coûte environ 3 RTT (TCP + TLS 1.3 + requête/réponse), soit **2 s** ; avec TLS 1.2, environ 2,5 s. Les messages sont petits (moins de 2 Ko), le débit de 256 kbps n'est donc pas le facteur limitant : c'est la latence.

| Délai | Valeur proposée | Raison |
|---|---|---|
| Connexion (TCP + TLS) | **10 s** | Largement au-dessus des ~1,2 s attendus sur satellite |
| Total de la requête | **20 s** | Marge pour une liaison dégradée ; au-delà, l'utilisateur doit savoir |
| `auth-service` côté Gateway | **5 s** | Au-delà → `503` |

**Pas de nouvel essai automatique** après un délai dépassé : un login n'est pas idempotent (chaque tentative compte pour la limitation de débit, voir §7). C'est l'utilisateur qui relance.

**Pas de redirection** : le Client refuse toute réponse `3xx` (suivre une redirection enverrait le mot de passe ailleurs). Une `3xx` donne `INVALID_RESPONSE`.

---

## 3. Requête

> Point d'accès complémentaire pour le E2E : `GET /health` (sans authentification) répond `200 {"status":"ok"}` quand la Gateway tourne **[Q9]**. Il permet au script de test d'attendre que la pile soit prête avant de lancer les scénarios.

| Élément | Valeur | |
|---|---|---|
| Méthode et chemin | `POST /auth/login` | MUST |
| Protocole | HTTP/1.1 minimum, sur **TLS ≥ 1.2** (1.3 recommandé) | MUST |
| `Host` | Nom de la Gateway (doit correspondre au certificat) | MUST |
| `Content-Type` | `application/json; charset=utf-8` | MUST |
| `Accept` | `application/json` | MUST |
| `X-Request-Id` | UUID généré par le Client, **renvoyé tel quel** par la Gateway et journalisé des deux côtés | SHOULD (très utile pour déboguer le E2E) |
| `User-Agent` | `SecureCloud-Client/<version>` | SHOULD |
| `Authorization` | **Absent** (route publique) | MUST |
| Taille maximale du corps | 2 Ko, sinon `413` | MUST |

**Corps (JSON)**

| Champ | Type | Règle | Remarque |
|---|---|---|---|
| `login` | string | 3 à 32 caractères parmi `a-z 0-9 _ - .` | Déjà **normalisé** par le Client (`UserLogin` : début/fin nettoyés, minuscules). La Gateway/auth **doit** refaire sa propre validation et normalisation, sans faire confiance au Client |
| `password` | string | 1 à 256 octets UTF-8 | **Jamais** modifié, ni nettoyé, ni normalisé (les espaces font partie du mot de passe) |
| `mfa_code` | string ou `null` | 6 chiffres | **Réservé** : le Client envoie `null` ou omet le champ en v0 |

```json
{ "login": "alice", "password": "Mot-de-passe-de-test", "mfa_code": null }
```

---

## 4. Réponse : succès

| Élément | Valeur |
|---|---|
| Statut | `200 OK` |
| `Content-Type` | `application/json; charset=utf-8` |
| `Cache-Control` | `no-store` (MUST : la réponse contient des secrets) |
| `X-Request-Id` | Celui de la requête |

| Champ | Type | Règle | Valide si… |
|---|---|---|---|
| `access_token` | string | Opaque (JWT côté serveur) | Non vide |
| `refresh_token` | string | Opaque | Non vide |
| `token_type` | string | Vaut `"Bearer"` | Égal à `Bearer` |
| `expires_in` | integer | Secondes de validité de `access_token` | > 0. Le Client calcule l'expiration avec **sa propre horloge** (`maintenant + expires_in`), ce qui évite tout décalage d'horloge entre poste et serveur |
| `user.id` | string | Identifiant stable | Non vide (→ `UserId`) |
| `user.login` | string | Login normalisé | Valide selon `UserLogin` |
| `user.role` | string | `ADMIN`, `COORD` ou `STD_USER` | Valeur connue, sinon `INVALID_RESPONSE` (jamais de rôle par défaut) |
| `user.created_at` | string | Optionnel, ISO 8601 UTC (`2026-10-01T08:00:00Z`) | Si absent : le Client utilise l'instant de connexion |

```json
{
  "access_token": "eyJ...",
  "refresh_token": "r-9f3...",
  "token_type": "Bearer",
  "expires_in": 900,
  "user": { "id": "u-123", "login": "alice", "role": "STD_USER",
            "created_at": "2026-10-01T08:00:00Z" }
}
```

Le Client **ignore** les champs inconnus (ajouts futurs sans casser la v0), mais **rejette** une réponse dont un champ obligatoire manque ou est mal typé (`INVALID_RESPONSE`).

---

## 5. Réponse : erreurs

Corps commun : `{ "error": { "code": "…", "message": "…" } }`. Le `message` est destiné aux **journaux** ; le Client affiche ses propres textes (traduisibles) selon le `code`.

| HTTP | `error.code` | Quand | Côté Client → `AuthError` | En-têtes |
|---|---|---|---|---|
| 400 | `MALFORMED_REQUEST` | JSON invalide, champ manquant ou mal typé, `login` hors format | `INVALID_INPUT` | |
| 401 | `INVALID_CREDENTIALS` | Identifiant inconnu **ou** mot de passe faux | `INVALID_CREDENTIALS` | |
| 401 | `MFA_REQUIRED` | Second facteur exigé (réservé, inutilisé en v0) | `INVALID_RESPONSE` (non géré en v0) | |
| 403 | `ACCOUNT_DISABLED` | Identifiants **corrects**, compte désactivé | `ACCOUNT_DISABLED` | |
| 413 | `PAYLOAD_TOO_LARGE` | Corps > 2 Ko | `INVALID_INPUT` | |
| 415 | `UNSUPPORTED_MEDIA_TYPE` | `Content-Type` ≠ JSON | `INVALID_RESPONSE` (bug Client) | |
| 429 | `RATE_LIMITED` | Trop de tentatives | `RATE_LIMITED` | **`Retry-After: <secondes>`** (MUST) |
| 500 | `INTERNAL_ERROR` | Erreur inattendue Gateway ou auth | `SERVICE_UNAVAILABLE` | |
| 502 / 503 / 504 | `SERVICE_UNAVAILABLE` | `auth-service` injoignable ou trop lent | `SERVICE_UNAVAILABLE` | `Retry-After` (SHOULD) |
| 3xx | — | Redirection | `INVALID_RESPONSE` (refus de suivre) | |
| Autre | — | Statut inattendu, ou corps non JSON | `INVALID_RESPONSE` | |

Règles de sécurité sur les erreurs :
- **MUST** : `401 INVALID_CREDENTIALS` identique, au caractère près, que l'identifiant existe ou non (pas d'énumération de comptes).
- **MUST** : même **temps de réponse** dans les deux cas (auth-service hache un mot de passe factice si l'identifiant est inconnu), sinon la durée trahit l'existence du compte.
- **MUST** : `403 ACCOUNT_DISABLED` n'est renvoyé qu'**après** vérification correcte du mot de passe ; sinon on révèle l'état d'un compte à n'importe qui.

---

## 6. Correspondance Gateway ↔ `auth-service` (gRPC)

À titre de guide pour l'équipe Gateway : un statut gRPC par cas, traduit en HTTP.

| Résultat de `auth-service.Login()` | Statut gRPC | → HTTP | `error.code` |
|---|---|---|---|
| Tokens émis | `OK` | 200 | — |
| Champ invalide | `INVALID_ARGUMENT` | 400 | `MALFORMED_REQUEST` |
| Identifiant inconnu / mot de passe faux | `UNAUTHENTICATED` | 401 | `INVALID_CREDENTIALS` |
| Compte désactivé | `PERMISSION_DENIED` | 403 | `ACCOUNT_DISABLED` |
| Quota dépassé | `RESOURCE_EXHAUSTED` | 429 | `RATE_LIMITED` |
| Service arrêté | `UNAVAILABLE` | 503 | `SERVICE_UNAVAILABLE` |
| Délai dépassé (5 s) | `DEADLINE_EXCEEDED` | 504 | `SERVICE_UNAVAILABLE` |
| Autre | `INTERNAL`, `UNKNOWN`… | 500 | `INTERNAL_ERROR` |

---

## 7. Règles côté Gateway pour `POST /auth/login`

| Règle | Niveau |
|---|---|
| Limitation de débit **avant** l'appel à `auth-service` (par login et par adresse IP). Proposition de départ : 5 échecs par login sur 5 minutes ⇒ `429` + `Retry-After` **[Q4]** | MUST |
| Ne **jamais** journaliser le corps de la requête ni les en-têtes `Authorization` ; journaliser `X-Request-Id`, le statut, `error.code`, la durée | MUST |
| `Cache-Control: no-store` sur toute réponse de `/auth/*` | MUST |
| Écouter en TLS uniquement (pas de HTTP clair, même en dev) | MUST |
| En-têtes de sécurité usuels (`X-Content-Type-Options: nosniff`) | SHOULD |
| Chaque tentative (réussie ou non) donne un `AuditEvent` (`USER_LOGIN` / échec) **sans** mot de passe ni token (AUD-01) | SHOULD |

---

## 8. Comptes de test pour le E2E **[Q3]**

Mots de passe à définir par l'équipe, **réservés au dev** (jamais dans un dépôt public ni une configuration de production).

| Login | Rôle | État | Sert à |
|---|---|---|---|
| `alice` | `STD_USER` | actif | E2E-LOGIN-01 (succès), E2E-LOGIN-02 (mauvais mot de passe) |
| `coord1` | `COORD` | actif | Vérifier le rôle `COORD` renvoyé |
| `admin1` | `ADMIN` | actif | Vérifier le rôle `ADMIN` renvoyé |
| `disabled1` | `STD_USER` | **désactivé** | `403 ACCOUNT_DISABLED` |
| `ghost` | — | **n'existe pas** | E2E-LOGIN-05 : doit répondre comme un mauvais mot de passe |

---

## 9. Vérifier le contrat à la main (`curl`)

L'équipe Gateway peut tester sans le Client :

```bash
# Succès attendu (200)
curl -v --cacert dev-ca.pem https://localhost:8443/auth/login \
  -H 'Content-Type: application/json' -H 'Accept: application/json' \
  -H 'X-Request-Id: 11111111-2222-3333-4444-555555555555' \
  -d '{"login":"alice","password":"<mot de passe de test>"}'

# Mauvais mot de passe attendu (401 INVALID_CREDENTIALS)
curl -i --cacert dev-ca.pem https://localhost:8443/auth/login \
  -H 'Content-Type: application/json' -d '{"login":"alice","password":"faux"}'

# Identifiant inconnu : doit renvoyer EXACTEMENT la même réponse que ci-dessus
curl -i --cacert dev-ca.pem https://localhost:8443/auth/login \
  -H 'Content-Type: application/json' -d '{"login":"ghost","password":"faux"}'
```

Ce faux serveur de test côté Client (`tests/tools/`) implémentera **ce même contrat**, ce qui permet de comparer les deux.

---

## 10. Réservé : hors E2E du 6/11

| Endpoint | Usage | Esquisse (à confirmer) |
|---|---|---|
| `POST /auth/refresh` | Renouveler l'`access_token` | Corps `{ "refresh_token": "…" }` → même forme que la réponse de login, sans `user` ; le refresh token est **tourné** (l'ancien devient invalide) |
| `POST /auth/revoke` | Révoquer des sessions (AUTH-01) | `Authorization: Bearer …` ; cible : soi-même (déconnexion) ou un autre utilisateur (permission `USER_REVOKE`) → `204` |
| `GET /auth/validate` | **Gateway ↔ auth uniquement** | Le Client ne l'appelle jamais |

---

## 11. Questions à l'équipe Gateway / auth-service

| # | Question | Proposition par défaut |
|---|---|---|
| Q1 | Chemin et port de dev ? | `https://localhost:8443/auth/login` (443 en prod) |
| Q2 | Durée de vie de l'`access_token` en démo ? | 900 s (15 min) |
| Q3 | Quels comptes de test préchargés dans `docker compose` ? | Tableau du §8 |
| Q4 | Politique anti-force-brute ? | 5 échecs / 5 min par login ⇒ `429` + `Retry-After` |
| Q5 | Quelle autorité de certification en dev, et où le Client récupère-t-il son certificat racine ? | CA maison `dev-ca.pem` fournie dans le dépôt de la Gateway, **sans sa clé privée** |
| Q6 | La Gateway renvoie-t-elle `X-Request-Id` ? | Oui |
| Q7 | Noms de champs en `snake_case` acceptés ? | Oui |
| Q8 | HTTP/1.1 suffisant côté Gateway (HTTP/2 non requis) ? | Oui |
| Q9 | `GET /health` disponible sur la Gateway ? | Oui |
| Q10 | Préfixe de version (`/v1/auth/login`) ou chemins de la spécification inchangés ? | Chemins inchangés en SKELETON |
| Q11 | `created_at` du profil disponible dans `auth-service` ? | Sinon on s'en passe (optionnel) |

---

## 12. Traçabilité

| Élément | Spec / ADR |
|---|---|
| REST externe, gRPC interne | ARC-01 |
| JWT standard, tokens courts + refresh | F3.2, AUTH-01 |
| Pas de crypto maison, TLS vérifié | SEC-02, AUTH-02 |
| Brute force : limitation de débit | Décision 45 (Vision École) |
| Audit sans secrets | AUD-01 |
| Latence satellite 256 kbps (scénario dégradé) | PERF-03, QT-03 |
