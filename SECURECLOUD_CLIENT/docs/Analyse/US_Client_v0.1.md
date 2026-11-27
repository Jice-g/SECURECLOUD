# User Stories : Client SecureCloud (v0.1)

> SECURECLOUD[SKELETON] V0.0 · Client Qt · **v0.1 du 27/11/2026** · statut **brouillon à relire** · Niveau 2 (Client)
> **Documents liés** : `UseCases.md` · `specifications_skeleton.md` · `specifications_roadmap.md` · `ADR_global.md` · Domaine v0.2 · `SC_MCD_Merise` (draft v0.1.0)
> **Rôle de ce document** : dire *ce que l'utilisateur veut faire* et *à quelles conditions on considère que c'est fait*. Il alimente directement la matrice US ↔ UC ↔ données (L2), les requêtes (L3) et le MCD (L4).

---

## 1. Mode d'emploi

**Chaîne de travail** : User Story → Use Case → besoins de données → requêtes → tables → repository → écran Qt → test. Le schéma de base de données se déduit des colonnes « Données » ci-dessous, jamais l'inverse.

**Format.** *En tant qu'**utilisateur**, je veux … afin de …*, suivi de critères d'acceptation « Étant donné / Quand / Alors ». Chaque critère doit devenir **au moins un test**.

**Vocabulaire fixé pour ce document.**

| Terme | Sens |
|---|---|
| **Session** | Utilisateur reconnu comme authentifié (état applicatif en mémoire ; jetons perdus au redémarrage). |
| **Réseau disponible / indisponible** | Le serveur est joignable ou non. **Distinct** de la session : on peut avoir une session ouverte sans réseau. |
| **Base locale** | SQLite du Client : cache du serveur + file d'envoi. Le serveur reste maître (D2). |
| **Message en attente** | Statut `BUFFERED` : validé par l'utilisateur, conservé en base, pas encore transmis. |

**Statuts.** **fait** · **SK** (V0.0) · **SK simplifié** · **base seule** (modélisé en base, inaccessible dans l'UI de test) · **RM** (reporté, voir roadmap).

**Décisions L0 appliquées** (27/11/2026) : alertes hors V0.0 (D1) ; brouillon perdu au redémarrage (D2) ; contenu local en clair en V0.0 (D5) ; groupes dans la base mais pas dans l'UI (D6) ; pas de suppression de message (D7) ; contacts en JSON hors SQLite (D8).

---

## 2. Vue d'ensemble

| US | Je veux… | UC | Statut | Lot de livraison proposé |
|---|---|---|---|---|
| US-01 | me connecter | UC-01 | **fait** | — |
| US-02 | me déconnecter | UC-09 | SK | A |
| US-03 | envoyer un message à un contact (y compris à moi-même) | UC-02 | SK | B |
| US-08 | voir la liste de mes conversations | UC-05 | SK | B |
| US-09 | lire l'historique d'une conversation, page par page | UC-05 | SK | B |
| US-07 | recevoir un message en temps réel | UC-06 | SK | C |
| US-04 | suivre l'état de mes messages | UC-02 | SK | C |
| US-05 | qu'un message écrit sans réseau parte au retour du réseau | UC-02 | SK | C |
| US-06 | relancer un message non remis | UC-15 | SK | C |
| US-10 | choisir la priorité d'un message | UC-02 | SK simplifié | D |
| US-12 | gérer mes contacts | UC-13 | SK simplifié | D |
| US-11 | rechercher un mot dans mon historique | UC-14 | SK | D |
| US-14 | joindre / télécharger un fichier | UC-04, UC-08 | **base seule** | — |
| US-13 | répondre à un message précis (thread) | UC-02 | **RM** (hors 1re version de test) | — |
| US-15 | voir si un contact est en ligne | — | **RM** | — |

Les lots A à D sont une **proposition d'ordre** (à valider) : A = cycle de session, B = cœur messagerie, C = fiabilité de l'envoi et réception, D = confort. Si le temps manque, le dernier lot est le premier sacrifié, jamais les tests.

**Nouveaux UC** : UC-13 (gérer ses contacts), UC-14 (rechercher dans l'historique), UC-15 (relancer un message non remis). Les UC existants gardent leur numéro.

---

## 3. Stories de la V0.0

### US-01 : Se connecter (UC-01) — **fait**

*En tant qu'utilisateur, je veux me connecter avec mon identifiant et mon mot de passe afin d'accéder à mes conversations.*

**Critères d'acceptation** (correspondent à E2E-LOGIN-01 à 05)
1. **Étant donné** un compte actif, **quand** je saisis un identifiant et un mot de passe corrects, **alors** une session est ouverte, mon rôle est connu, et la fenêtre principale s'affiche.
2. **Quand** le mot de passe est faux **ou** l'identifiant inconnu, **alors** le même message s'affiche dans les deux cas (« Identifiant ou mot de passe incorrect »).
3. **Quand** le compte est désactivé (et le mot de passe correct), **alors** « Compte désactivé, contactez votre administrateur ».
4. **Quand** le serveur répond « trop de tentatives », **alors** le message indique la durée d'attente si elle est connue.
5. **Quand** le serveur est injoignable (DNS, TCP, TLS, délai de 20 s), **alors** « Serveur inaccessible ».
6. **Quand** un champ est vide, ou le mot de passe dépasse 256 octets, **alors** aucun appel réseau n'est émis.
7. Le mot de passe n'apparaît dans aucun journal ; le champ est vidé après envoi.

**Données** : aucune table (session en mémoire). **Écran** : fenêtre de connexion.

---

### US-02 : Me déconnecter (UC-09) — SK

*En tant qu'utilisateur, je veux me déconnecter afin de fermer ma session sans perdre mon activité passée.*

**Critères d'acceptation**
1. **Étant donné** une session ouverte, **quand** je me déconnecte, **alors** la session est fermée, aucun jeton ne subsiste en mémoire et l'écran de connexion s'affiche.
2. **Alors** les fonctions qui exigent une session (envoyer, recevoir) ne sont plus accessibles.
3. **Alors** la **base locale est conservée** : à la reconnexion avec le même compte, je retrouve toutes mes conversations et mes messages.
4. **Quand** je me reconnecte avec un autre compte, **alors** je ne vois **que** les données de ce compte (isolement par utilisateur ; voir point ouvert P2).
5. Les messages `BUFFERED` en attente restent en base et sont conservés pour la prochaine session.
6. La révocation côté serveur est **hors périmètre** (RM) : la déconnexion est locale.

**Données** : aucune écriture, sauf la présence locale (non persistée). **Écran** : action « Se déconnecter » dans la fenêtre principale.

---

### US-03 : Envoyer un message à un contact (UC-02) — SK

*En tant qu'utilisateur, je veux envoyer un message à un autre utilisateur, ou à moi-même, afin de communiquer ou de garder des notes.*

**Critères d'acceptation**
1. **Étant donné** une session ouverte et un destinataire avec lequel je n'ai pas encore de conversation, **quand** j'envoie « Bonjour », **alors** une conversation directe est créée au premier message (BR-08), et le message apparaît dans le fil.
2. **Quand** j'envoie un second message au même destinataire, **alors** il est ajouté à la **même** conversation (pas de doublon, BR-07).
3. **Quand** je m'envoie un message à moi-même, **alors** la conversation est acceptée (self-chat) et il en existe **une seule** pour moi-même.
4. **Quand** le message est vide ou dépasse 4096 octets, **alors** il est refusé avec un message clair, sans appel réseau (BR-10).
5. **Quand** je tente d'envoyer vers un groupe, **alors** l'envoi est refusé avec « fonction non disponible » (BR-21). *(Cas de test : la fonction n'est pas accessible dans l'UI, ce critère se teste au niveau Application.)*
6. Le message porte un identifiant généré **côté Client** (idempotence, BR-13) et un horodatage.
7. **Sans session**, aucune action d'envoi n'est possible (BR-01).
8. Un message en cours de rédaction (brouillon) **n'est pas conservé** au redémarrage (D2).

**Données** : écrit `conversation` (si absente), `participant` (×2, ou ×1 si self-chat), `message` (statut initial `BUFFERED`, `priority = NORMAL`). **Écran** : fil de conversation + zone de saisie.

---

### US-08 : Voir la liste de mes conversations (UC-05) — SK

*En tant qu'utilisateur, je veux voir la liste de mes conversations afin de retrouver rapidement un échange.*

**Critères d'acceptation**
1. **Étant donné** plusieurs conversations, **quand** j'ouvre la liste, **alors** chacune affiche l'interlocuteur, un aperçu du dernier message et sa date, triées de la plus récente à la plus ancienne.
2. Les conversations d'un autre compte ne sont **jamais** affichées.
3. La liste est disponible **sans réseau** (lecture de la base locale).
4. **Étant donné** aucune conversation, **alors** un état vide explicite s'affiche.
5. Une conversation de type groupe, si elle existait en base, **n'apparaît pas** (D6).
6. Le self-chat apparaît avec un libellé explicite (« Moi »).

**Données** : lit `conversation` + `participant` + dernier `message`. **Écran** : liste de conversations.

---

### US-09 : Lire l'historique d'une conversation (UC-05) — SK

*En tant qu'utilisateur, je veux lire l'historique d'une conversation, page par page, afin de retrouver ce qui a été dit sans charger toute la base.*

**Critères d'acceptation**
1. **Quand** j'ouvre une conversation, **alors** les messages les plus récents s'affichent en premier (taille de page proposée : 50, à confirmer en L3).
2. **Quand** je remonte dans le fil, **alors** la page précédente est chargée ; aucun message n'est dupliqué ni sauté.
3. L'historique est consultable **sans réseau** (activité passée accessible hors connexion).
4. Chaque message affiche l'expéditeur, le contenu, l'horodatage, la priorité et, pour mes messages, son statut.
5. L'ordre est stable pour deux messages de même horodatage (critère de tri secondaire à définir en L3).
6. La Conversation n'embarque **pas** ses messages en mémoire (le domaine ne garde que l'identifiant).

**Données** : lit `message` filtré par `conversation`, trié, avec pagination (curseur ou décalage : L3). **Écran** : fil de conversation.

---

### US-07 : Recevoir un message en temps réel (UC-06) — SK

*En tant qu'utilisateur, je veux recevoir les messages qui me sont destinés dans la bonne conversation afin de répondre sans quitter l'application.*

**Critères d'acceptation**
1. **Étant donné** une session ouverte et le réseau disponible, **quand** un message m'est destiné, **alors** il apparaît dans la conversation correspondante et la liste est mise à jour.
2. **Quand** l'expéditeur m'écrit pour la première fois, **alors** la conversation est créée localement avec le même identifiant que celui du serveur.
3. **Quand** le même message est reçu deux fois (même identifiant), **alors** il n'est enregistré **qu'une fois** (idempotence).
4. Je suis informé de la réception (notification simple, désactivable).
5. Le contenu reçu est stocké en base en clair en V0.0 (D5) ; le schéma prévoit un champ binaire.
6. **Étant donné** une session fermée, **quand** un message arrive côté serveur, **alors** il sera récupéré à la prochaine connexion *(la forme de cette récupération dépend du contrat Messaging ; voir point ouvert P5)*.

**Données** : écrit `message` (+ `conversation` / `participant` si inconnue). **Écran** : fil, liste, notification.

---

### US-04 : Suivre l'état de mes messages (UC-02) — SK

*En tant qu'utilisateur, je veux voir l'état de chaque message que j'ai envoyé afin de savoir s'il est parti, arrivé ou en échec.*

**Critères d'acceptation**
1. **Quand** je valide un message, **alors** il est « en attente » (`BUFFERED`), puis « en cours » (`SENDING`) pendant la transmission, puis « envoyé » (`SENT`) lorsque le serveur l'a accusé.
2. **Quand** la transmission échoue, **alors** il passe à « non remis » (`UNDELIVERED`) ; la cause (connexion ou serveur) est conservée.
3. Le statut ne progresse que dans le sens prévu ; **seule** la reprise `UNDELIVERED` → `BUFFERED` revient en arrière (BR-12).
4. **Au redémarrage**, un message resté `SENDING` repasse en `BUFFERED` (jamais perdu, jamais bloqué).
5. Les statuts `DELIVERED` et `READ` sont **optionnels en V0.0** : l'UI les affiche s'ils arrivent, ne les exige pas. *Le sens exact de `SENT` / `DELIVERED` est à figer avec l'équipe serveur (MSG-06, point ouvert P4).*
6. Chaque statut est affiché par une icône ou un libellé distinct.

**Données** : lit/écrit `message.status`, `buffered_at`, `sent_at`, cause d'échec. **Écran** : indicateur de statut sur chaque message envoyé.

---

### US-05 : Écrire sans réseau, envoi au retour du réseau (UC-02) — SK

*En tant qu'utilisateur, je veux que mes messages écrits sans réseau soient conservés et partent dès que le réseau revient afin de ne rien perdre sur le terrain.*

**Critères d'acceptation**
1. **Étant donné** une session ouverte et le réseau indisponible, **quand** j'envoie un message, **alors** il est enregistré en base avec le statut « en attente » et visible dans le fil.
2. **Quand** le réseau revient, **alors** les messages en attente partent dans l'**ordre de leur création**.
3. Chaque message est envoyé **une seule fois** même si la transmission est relancée (idempotence par identifiant, BR-13).
4. **Quand** l'application redémarre avec des messages en attente, **alors** ils sont toujours là et seront envoyés à la prochaine session.
5. Un message déjà accepté par le serveur n'est jamais renvoyé comme nouveau.
6. L'état « hors réseau » est signalé à l'utilisateur de manière visible.

**Données** : lit `message` `BUFFERED` triés par `buffered_at` ; écrit les statuts. **Écran** : indicateur de connexion.
**Test** : coupure réseau simulée (FILE-01, F2.7 version messages).

---

### US-06 : Relancer un message non remis (UC-15, nouveau) — SK

*En tant qu'utilisateur, je veux relancer un message qui n'a pas pu partir afin de ne pas devoir le réécrire.*

**Critères d'acceptation**
1. **Étant donné** un message `UNDELIVERED`, **quand** je demande de le renvoyer, **alors** il repasse en « en attente » et suit le circuit d'envoi normal.
2. Le contenu et l'identifiant du message sont **inchangés** (pas de doublon côté serveur).
3. La relance n'est proposée **que** pour les messages en échec.
4. **Quand** la relance échoue de nouveau, **alors** le message revient à « non remis » sans perte.

**Données** : `message.status` : `UNDELIVERED` → `BUFFERED`. **Écran** : action « Renvoyer » sur un message en échec.

---

### US-10 : Choisir la priorité d'un message (UC-02) — SK simplifié

*En tant qu'utilisateur, je veux indiquer la priorité d'un message (normale, haute, critique) afin de signaler une urgence à mon correspondant.*

**Critères d'acceptation**
1. **Par défaut**, un message est `NORMAL`.
2. **Quand** je choisis `HIGH` ou `CRITICAL`, **alors** la priorité est conservée avec le message et visible du destinataire.
3. **Quand** je reçois un message `CRITICAL`, **alors** il se distingue visuellement (et, si possible, par un son) de façon nette.
4. La priorité est **logique** : elle n'est jamais présentée comme une garantie de livraison instantanée (MSG-03).
5. La priorité est visible du serveur (elle sert au routage) ; le contenu ne l'est pas.

**Données** : `message.priority` (`NORMAL`, `HIGH`, `CRITICAL`). **Écran** : sélecteur de priorité à côté de la saisie.

---

### US-12 : Gérer mes contacts (UC-13, nouveau) — SK simplifié

*En tant qu'utilisateur, je veux gérer un carnet de contacts (nom d'affichage, notes) afin de retrouver facilement mes correspondants.*

**Critères d'acceptation**
1. **Quand** j'ajoute un contact par son identifiant, **alors** une fiche `[userId].json` est créée avec nom d'affichage et notes.
2. **Quand** je modifie ou consulte la fiche, **alors** les changements sont conservés au redémarrage.
3. **Quand** une fiche est absente ou illisible, **alors** l'application affiche l'identifiant (ou le login) à la place, sans planter.
4. Les contacts **ne font pas partie du domaine** (vue personnalisée d'un autre utilisateur, côté Application) et ne sont **pas** dans SQLite (D8).
5. Les fiches sont **par compte** : un autre utilisateur du poste ne voit pas mes contacts.
6. Le nom d'affichage est utilisé dans la liste des conversations et le fil (US-08, US-09).

**Données** : fichiers JSON (une fiche par `userId`). **Écran** : carnet de contacts. *(Preuve RNCP : accès NoSQL.)*

---

### US-11 : Rechercher dans mon historique (UC-14, nouveau) — SK

*En tant qu'utilisateur, je veux rechercher un mot dans mes messages afin de retrouver une information passée.*

**Critères d'acceptation**
1. **Quand** je saisis un mot, **alors** la liste des messages qui le contiennent s'affiche, avec leur conversation et leur date.
2. La recherche est **entièrement locale** : aucun appel réseau, aucun moteur de recherche côté serveur (SEC-04).
3. Elle fonctionne **sans réseau**.
4. **Quand** aucun résultat, **alors** un message explicite s'affiche.
5. Seuls mes messages (mon compte) sont cherchés.
6. **Limite assumée** : en V0.0 le contenu est stocké en clair (D5), donc la recherche porte directement sur la base ; une fois le stockage chiffré, il faudra un **index local** (Roadmap, `Crypto.md`).

**Données** : lit `message.content`. **Écran** : champ de recherche + résultats.

---

## 4. Stories hors première version de test

### US-14 : Joindre et télécharger un fichier (UC-04, UC-08) — **base seule**

*En tant qu'utilisateur, je veux joindre un fichier à un message et télécharger celui que je reçois afin d'échanger des documents.*

**Décision (27/11)** : la table des pièces jointes **existe** dans le MCD/MLD/MPD ; la fonction n'est **pas accessible** dans l'UI de test primaire : le bouton « Joindre » est présent mais `setEnabled(false)`.

**Critères d'acceptation de la V0.0**
1. La table `attachment` (métadonnées : identifiant, nom, taille, type, rattachement à un message) est créée par le schéma.
2. Un test d'accès aux données prouve qu'on peut insérer et relire une pièce jointe liée à un message existant (une pièce jointe n'existe jamais sans message, BR-14).
3. Dans l'UI, le bouton est désactivé et **aucun** chemin de code ne peut l'activer.
4. Aucun transfert réel (pas de chunking, pas de compression) en V0.0.

**Point ouvert** : le nom de fichier est-il en clair pour le serveur ou inclus dans le contenu opaque (FILE-06, O-1) ? À trancher avant d'activer la fonction ; cela peut modifier la colonne `file_name` du schéma.

### US-13 : Répondre à un message précis (thread) — **RM** (hors première version de test)

*En tant qu'utilisateur, je veux répondre à un message précis afin de garder le fil d'une discussion.*

**Raison du report** : la fonction génère du code d'interface Qt non nécessaire à la validation du fonctionnement primaire. Spécification d'origine : F1.5, MSG-01 (thread = réponse simple, sans arborescence).

**Critères d'acceptation (pour plus tard)** : le message répondu est visible en citation ; un message peut référencer **un seul** parent ; un parent supprimé ou absent est toléré sans plantage.
**Question pour L4** : prévoir dès maintenant une colonne `parent_message_id` nullable (coût quasi nul, évite une migration) ? Proposition : **oui, dans le MLD, inutilisée en V0.0**.

### US-15 : Voir si un contact est en ligne — **RM**

*En tant qu'utilisateur, je veux savoir si un contact est en ligne afin de choisir comment le joindre.*

Reporté. Aucune table : la présence suit la session (BR-20, MSG-02) et n'est pas conservée en base. `AWAY` reste réservé à la V1 (RM-03).

### Reportées (RM), non modélisées en UI

Groupes de discussion (UC-03, UC-07, RM-01) · alertes, groupes d'alerte, acquittements (UC-10, UC-11, UC-12, D1) · suppression d'un message (D7) · modification du login · état de lecture par destinataire · MFA, récupération de mot de passe (RM-12, RM-14).

---

## 5. Traçabilité

| US | UC | Spécification / ADR | Règles métier |
|---|---|---|---|
| US-01 | UC-01 | F3.1, F3.7, AUTH-01, AUTH-06, ARC-01 | BR-01, BR-11 |
| US-02 | UC-09 | F3 (déconnexion locale), MSG-02 | BR-01, BR-02, BR-20 |
| US-03 | UC-02 | F1.1, F1.11, MSG-04, MSG-07 | BR-01, BR-06, BR-07, BR-08, BR-10, BR-13, BR-21 |
| US-04 | UC-02 | F1.10, MSG-06 | BR-12, BR-13 |
| US-05 | UC-02 | F2.7 (principe), FILE-01 | BR-12, BR-13 |
| US-06 | UC-15 | F1.10, MSG-06 | BR-12, BR-13 |
| US-07 | UC-06 | F1.3, ARC-02, SEC-03 | BR-01, BR-09, BR-13 |
| US-08 | UC-05 | MSG-04, MSG-07 | BR-06, BR-07 |
| US-09 | UC-05 | — | BR-06, BR-09 |
| US-10 | UC-02 | F1.8, MSG-03 | BR-09 |
| US-11 | UC-14 | F1.6, SEC-04 | BR-09 |
| US-12 | UC-13 | D8 (contacts hors domaine) | — |
| US-14 | UC-04, UC-08 | F2.8, FILE-02, FILE-06 | BR-14 |
| US-13 | UC-02 | F1.5, MSG-01, RM-04 | — |
| US-15 | — | F1.4, MSG-02, RM-03 | BR-20 |

---

## 6. Points ouverts avant le MCD (L4)

| # | Question | Impact | Proposition |
|---|---|---|---|
| P1 | La `Conversation` possède-t-elle un champ `type` (`DIRECT` / `GROUP`) ? | MCD, contraintes `CHECK` | Oui (D6 : groupes dans la base). |
| P2 | **Plusieurs comptes sur un même poste** : une base par utilisateur, ou une colonne propriétaire dans chaque table ? US-02 critère 4 en dépend. | Chemin du fichier SQLite, clés, requêtes | Une base par `UserId` (fichier distinct) : isolement simple, aucune colonne en plus. |
| P3 | **Accès à l'historique sans réseau après redémarrage** : le login exige le serveur (contrat v0.1), donc sans réseau on ne peut pas ouvrir de session. | Sécurité, UI de démarrage | À décider : (a) l'historique n'est lisible qu'**après** une connexion réussie ; (b) mode lecture seule local sans session. Avec le contenu en clair (D5), (b) expose les messages à quiconque ouvre le poste : à documenter comme limite si retenu. |
| P4 | Sens exact de `SENT` / `DELIVERED` et `READ` (MSG-06). | Statuts persistés, contrat | Recommandation MSG-06 (lecture A) ; à figer avec l'équipe serveur. |
| P5 | Récupération des messages reçus pendant que la session est fermée (synchronisation à la connexion). | Contrat Messaging, requêtes | Hors schéma si le serveur renvoie les messages en attente ; sinon prévoir un marqueur de dernière synchronisation. |
| P6 | Clés primaires : Id du domaine (TEXT) avec `login UNIQUE` (D3). | MLD, alignement PostgreSQL | Recommandé ; format des Id à confirmer avec le serveur. |
| P7 | Conversation directe : identifiant dérivé de la paire triée ou UUID (D4, O-10) ? | Clé de `conversation`, unicité | `UNIQUE` sur la paire canonique en attendant. |
| P8 | `parent_message_id` dès le MLD pour US-13 ? | Schéma | Oui, nullable, inutilisé. |
| P9 | Taille de page de l'historique et critère de tri secondaire (US-09). | Requête paginée | 50 messages ; tri par horodatage puis identifiant. |

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 0.1 | 27/11/2026 | Première version : 15 stories avec critères d'acceptation, arbitrages L0 appliqués (D1 à D8), arbitrages US-13/14/15/02, points ouverts avant L4 |
