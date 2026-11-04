# Correspondance des anciens identifiants

> **Version** 1.0 · **Date** 04/11/2026 · **Statut** brouillon à relire · **Niveau** 1 (commun)
> **Documents liés** : [ADR_global.md](ADR_global.md)

**Principe : on repart au propre.** Ce document n'est pas un inventaire exhaustif. Il sert uniquement à **retrouver** une décision quand on lit un document ancien (`General - Descript`, spécification v1, domaine v0.2, `UC1_Connexion.md`, `Login_features.md`). Les familles d'identifiants abandonnées ne sont pas remappées une à une.

---

## 1. Identifiants thématiques conservés

Ces identifiants existent toujours **à l'identique** dans `ADR_global.md`, sauf mention :

`SEC-01`, `SEC-02`, `SEC-03`, `ARC-01` à `ARC-05`, `CPP-01`, `AUTH-02`, `AUTH-03`, `AUTH-04`, `FILE-01`, `FILE-02`, `FILE-03`, `MSG-01` à `MSG-05`, `AUD-01`, `PERF-01` à `PERF-03`, `OPS-01`, `QT-01` à `QT-03`, `CI-01`, `CI-02`, `TEST-01`, `TEST-02`.

## 2. Identifiants modifiés

| Ancien | Nouveau | Nature du changement |
|---|---|---|
| `ARC-06` | `ARC-06` | **Statut corrigé** : DS dans la spécification v1, « ?? » dans une copie → **RM** |
| `AUTH-01` | `AUTH-01` + `AUTH-05` | **Scindé** : le modèle de jetons reste `AUTH-01` ; la définition de la révocation « immédiate » devient `AUTH-05` |
| `IPC-01` | `ARC-01` | **Remplacé** : l'abstraction de transport (socket de domaine) cède la place à gRPC |
| `SEC-02` (extension « VFS ») | `FILE-05` | Le stockage fichier par fichier devient une décision propre |
| Compression avant chiffrement (F2.3, sans identifiant) | `FILE-04` | Identifiant créé |
| Recherche locale (F1.6, sans identifiant) | `SEC-04` | Identifiant créé |
| « Chemin critique », audit asynchrone (analyse n° 42) | `ARC-07` | Identifiant créé |

## 3. Ponts vers les décisions issues du travail récent

Décisions des documents de travail du Client qui concernent **les deux niveaux** : elles sont portées dans le registre global.

| Ancien repère | Nouveau | Sujet |
|---|---|---|
| `D-L8`, décision ③ de `UC1_Connexion.md`, point `O-8` | `AUTH-06` | Normalisation du login en minuscules |
| Décision ② de `UC1_Connexion.md`, point `O-9` | `AUTH-03` | Trois niveaux de nommage des rôles |
| Décision 45 de la Vision école | `AUTH-07` | Résistance à la force brute |
| Point `O-1` du domaine v0.2 | `FILE-06` | Nom de fichier d'une pièce jointe |
| Points `O-5`, `O-6`, point `R3` de la feuille de route Messaging | `MSG-06` | Statuts de livraison, idempotence, message reçu |
| Points `R5`, `O-10` | `MSG-07` | Création de la conversation, identifiants générés par le Client |
| Point `R9`, règle `BR-21` | `MSG-04` | Envoi vers un groupe refusé explicitement |
| Point `O-4` | `MSG-05` (point ouvert), `RM-06` | Relance après non-acquittement |

## 4. Familles abandonnées

| Famille | Devenir |
|---|---|
| Les 46 constats numérotés de la Vision école | Consolidés **par thème** dans `school_vision/analyse_cahier_des_charges.md`, sans numéro |
| Classes `C1` à `C4` | Remplacées par les statuts de proposition (`projet_et_rncp.md` §7.2) et le statut SK / RM / DS |
| `D1` à `D15` (mémo de la Scrum 2) | Retirés. Contradictions déjà relevées dans la feuille de route Messaging (R10) ; le mémo sera refait au propre au niveau Client |
| `D-L1` à `D-L12`, `CLIENT-01` à `CLIENT-05` | Décisions propres au Client : reprises dans le registre des décisions du **Client** (lot suivant) |
| `R1` à `R11` (feuille de route Messaging) | Dette technique du Client : restent dans la documentation du Client |
| `O-1` à `O-10` (points ouverts du domaine v0.2) | Restent dans le document de domaine ; les points transverses sont dans la section 3 |
| `BR-01` à `BR-21` (règles métier) | Restent dans le document de domaine |

## 5. Convention de citation

Dans un document, citer un identifiant sous la forme **`ADR SEC-02`** ou `SEC-02`. Un identifiant n'est **jamais réutilisé** : une décision remplacée garde son identifiant et son statut « remplacé ».

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 1.0 | 04/11/2026 | Création |
