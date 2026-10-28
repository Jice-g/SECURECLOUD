# DOMAIN.md — SECURECLOUD[SKELETON]
> Modèle du domaine métier. Ce document précède et gouverne l'architecture technique.
> Toute décision d'architecture qui contredit une règle définie ici doit revenir ici d'abord — pas l'inverse.
>
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
