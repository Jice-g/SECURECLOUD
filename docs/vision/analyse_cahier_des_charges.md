# Analyse critique du cahier des charges

> **Version** 1.0 · **Date** 04/11/2026 · **Statut** brouillon à relire · **Niveau** 1 (commun)
> **Documents liés** : [prototype_skeleton.md](prototype_skeleton.md) · [../specifications/specifications_skeleton.md](../specifications/specifications_skeleton.md) · [../decisions/ADR_global.md](../decisions/ADR_global.md)

Ce document explique **pourquoi** certaines exigences du cahier des charges ont dû être interprétées, précisées, reportées ou écartées. Il ne décide rien : les décisions sont dans le registre (`ADR_global.md`), les exigences retenues dans les spécifications. Les identifiants de la colonne « Décision » renvoient au registre.

---

## 1. Sources et hiérarchie

| Document | Nature | Poids |
|---|---|---|
| `SecureCloud - Cahier des charges.docx` | Référentiel officiel du projet | **Fait foi** ; toute modification doit être validée par l'équipe et le client |
| `SecureCloud.pdf` | Mise en situation narrative reprenant et développant le cahier | Complète, ne contredit pas |

Contexte de travail : trois étudiants, six semaines effectives, contraintes d'usage extrêmes. Cela se traduit en **recherche des moyens de renforcer la fiabilité**, tout en limitant ce que l'on prétend prouver.

## 2. Verdict d'ensemble

> Le cahier des charges est **suffisamment précis pour définir le produit**, mais **pas pour définir l'architecture finale**. Le travail de l'équipe consiste précisément à combler cet écart de façon traçable.

Chaîne de traçabilité visée : **cahier des charges → contradiction ou ambiguïté → décision d'équipe → spécification → architecture → implémentation → test.**

## 3. Les quatre points majeurs

| # | Point | Nature | Traitement |
|---|---|---|---|
| 1 | L'API REST d'Auth est consommée par les services, mais « pas de HTTP entre services » | **Contradiction** | REST à l'extérieur, **gRPC** à l'intérieur (ARC-01) |
| 2 | Upload de 500 Mo en moins de 3 minutes **et** liaison satellite de 256 kbps | **Contradiction physique** : 500 Mo à 256 kbps prennent plus de 4 heures ; tenir 3 minutes exige au moins 22 Mbit/s | Deux scénarios distincts, jamais présentés comme simultanés (PERF-03) |
| 3 | « Inviolable », « impossibilité d'interception » | **Non démontrable** | Vision conservée ; exigence remplacée par un modèle de menace (SEC-01) |
| 4 | « Zéro tolérance à la panne » avec « 99,9 % de disponibilité » | **Contradiction** de formulation ; Docker Compose seul n'apporte pas de redondance | « Zéro panne » supprimé ; 99,9 % devient un objectif d'architecture cible (OPS-01) |

Ces quatre points sont traités **explicitement dans la documentation**, pas corrigés discrètement dans le code.

## 4. Constats par thème

Statuts : voir `projet_et_rncp.md` §7.2. « SK / RM / DS » : voir `prototype_skeleton.md` §2.

### 4.1 Sécurité

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| « AES-256 partout » | AES est un chiffrement symétrique : manquent génération, distribution, rotation, stockage des clés, nonces, authentification, intégrité, révocation | À arbitrer | SEC-02 : spécification cryptographique dédiée ; aucune primitive « maison » |
| Le serveur « ne voit jamais le contenu » | Le routage exige des métadonnées (expéditeur, destinataires, taille, horodatage) | À arbitrer | SEC-03 : le **contenu** est opaque, les métadonnées de routage sont explicitement hors garantie |
| Recherche dans l'historique | Impossible côté serveur si celui-ci ne lit rien | Compatible, à préciser | SEC-04 : recherche **exclusivement locale** |
| « JWT custom », « bcrypt optimisé » | Invention de protocole ou de primitive | Contradictoire avec « pas de crypto maison » | AUTH-01, AUTH-02 : bibliothèques éprouvées, formats standards |
| Authentification multi-facteurs | Aucun facteur défini | À arbitrer | AUTH-02 : TOTP ou MFA simulé, documenté |
| Révocation « immédiate » des jetons | Un JWT autoportant reste valide jusqu'à expiration | À arbitrer | AUTH-05 : redéfinition et mécanisme explicite |
| Permissions « granulaires » | Aucune matrice | À arbitrer | AUTH-03 : RBAC à 3 rôles |
| Résistance à la force brute | Attaque, seuil et mesure non définis | À arbitrer | AUTH-07 : limitation de débit mesurable |
| Auth centralisée | « Le serveur ne sait rien » serait faux : il connaît l'identité | À arbitrer | SEC-03 : trois catégories de données (nécessaires au service / sensibles chiffrées / contenu opaque) |
| Contenu de PostgreSQL | Rien ne dit ce qui est stocké | À arbitrer | SEC-03 : jamais de contenu en clair |
| Audit « complet », logs de « toutes les actions » | Risque d'enregistrer contenus, jetons, données médicales | À arbitrer | AUD-01 : événements de sécurité et d'administration seulement |
| « Audit externe » du chiffrement | Hors portée d'un projet étudiant | Hors périmètre | TEST-02 : revue interne documentée ; mention explicite de l'absence d'audit |

### 4.2 Architecture et déploiement

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| WebSocket pour la messagerie | Cohérent avec l'IPC interne seulement si l'on distingue extérieur et intérieur | Compatible | ARC-02 |
| L'API Gateway est-elle un 6e service ? | Elle est dans l'architecture mais pas dans la liste des 5 microservices | À arbitrer | ARC-03 : composant d'infrastructure distinct |
| Compose : liste sans la Gateway | Incohérence de nomenclature | À arbitrer | ARC-03 : « 4 services métier + 1 Gateway + 1 base de données » en SKELETON |
| `deploy-service` | Un « service de déploiement » existe en doublon du CI/CD | À arbitrer | ARC-06 : RM ; son rôle est assuré par CI/CD et Compose |
| 45 000 utilisateurs et PostgreSQL unique | « Database Cluster » cité, mais une seule instance en Compose | Non démontrable | ARC-04, AUTH-04 |
| Docker Compose et haute disponibilité | Pas de redondance, failover, répartition de charge | Contradictoire | ARC-05 : mono-nœud en SKELETON ; architecture cible réplicable |
| Named pipes, sockets de domaine, mémoire partagée | Portabilité et complexité ; conteneurs Linux ; gRPC retenu le 16/09/2026 | Contradictoire | ARC-01 |
| Serveur HTTP « natif » du service Files | Rappel de la règle « pas de HTTP interservices » | À arbitrer | ARC-01 : HTTP seulement en interface externe |
| Temps réel critique et appels interservices | Chaque service ajouté sur le chemin critique ajoute de la latence et un point de panne | À arbitrer | ARC-07 : audit asynchrone, autorisation en cache |
| C++ pour toute la pile | Les outils périphériques n'ont pas à l'être | Compatible | CPP-01 |

### 4.3 Performance et réseau

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| 10 000 messages/s | Ni nombre de clients, ni taille des messages, ni matériel, ni latence, ni chiffrement inclus ou non : pas un benchmark reproductible | Non démontrable | PERF-01 : scénario défini, valeur **mesurée** publiée |
| Temps de réponse < 100 ms | De quoi parle-t-on ? Moyenne ? Quel trajet ? | À arbitrer | PERF-02 : Client A → Client B, P95/P99 |
| Validation de jeton rapide | Même problème de métrique | À arbitrer | PERF-02 |
| « Sauvegarde sans Internet » | Sauvegarder où ? | À arbitrer | FILE-01 : stockage local chiffré, file d'attente, synchronisation différée |

### 4.4 Messagerie

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| Groupes jusqu'à 100 participants | Diffusion à N destinataires, permissions dynamiques, gestion des clés de groupe, tests multipliés | Hors périmètre | MSG-04 : 1-to-1 en SK, groupes en RM |
| Threads de conversation | Modèle non défini | À arbitrer | MSG-01 : thread = réponse à un message |
| Statuts de présence « sécurisés » | Qui voit quoi ? Historique ? | À arbitrer | MSG-02 : `ONLINE` / `OFFLINE` ; `AWAY` en RM |
| Notifications « intelligentes » | Non testable | À arbitrer | MSG-02 : règles explicites |
| Messages à « priorité absolue » | Niveaux, préemption, garanties non définis | À arbitrer | MSG-03 : trois niveaux logiques, aucune garantie d'infrastructure |
| « Message immédiat à toutes les équipes » | Définition d'une équipe, accusés, absents | À arbitrer | MSG-05 : groupe d'alerte distinct du groupe de discussion, suivi d'acquittement |
| Statuts d'un message envoyé (`SENT`, `DELIVERED`…) | Deux lectures coexistent | À arbitrer | MSG-06 |
| Où naît une conversation ? | Client et serveur peuvent se désynchroniser | À arbitrer | MSG-07 |
| Scénario « envoi d'un fichier dans un chat » | Messaging orchestre Auth, Files et Audit : couplage fort, que se passe-t-il si l'un tombe ? | À arbitrer | ARC-07 ; comportement en panne à documenter et à tester |

### 4.5 Fichiers

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| Déduplication au niveau des blocs | Deux fichiers identiques chiffrés avec des nonces différents donnent des blocs différents | Contradictoire avec E2E | FILE-02 : DS |
| Compression zlib + chiffrement | Ordre non défini ; des données chiffrées ne se compressent pas | À arbitrer | FILE-04 : compression **avant** chiffrement |
| Chunking « automatique » | Pas de taille, numérotation, vérification, reprise | À arbitrer | FILE-03 : protocole minimal défini |
| Système de fichiers virtuel chiffré « custom » | Hors proportion | Hors périmètre | FILE-05 : stockage chiffré fichier par fichier |
| Nom d'un fichier joint | « Rapport_Evacuation_Kaboul.pdf » peut révéler une information sensible | À arbitrer | FILE-06 |

### 4.6 Tests, CI/CD et qualité

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| Couverture de tests 100 % | Lignes ? Branches ? Cas d'usage ? | À arbitrer | TEST-01 |
| Pipeline à « chaque commit » (vision) contre « après une action définie » (cahier) | Pas équivalent ; le déploiement automatique en production à chaque commit est à éviter | À arbitrer | CI-01 |
| `git push` → 5 minutes → « production » | Environnement, matériel de CI, durée par étape non définis | À arbitrer | CI-02 : environnement de démonstration |
| Déploiement automatisé et sécurité | Pas de staging, de signature, de secrets, de rollback, de contrôle d'images | À arbitrer | CI-01 |
| Démarrage < 2 s, RAM < 50 Mo | Comment mesurer sans suite de benchmark ? | À arbitrer | QT-02 : mesure ponctuelle |
| 45 °C, poussière, matériel vieillissant | Ne peut pas être testé en école | Non démontrable | QT-03 : seul le réseau dégradé est simulé |
| Multiplateforme | Pas de machine macOS dans l'équipe | Non démontrable | QT-01 : Windows et Linux ; macOS *best effort* |

### 4.7 Cadre

| Exigence du cahier | Problème | Statut | Décision |
|---|---|---|---|
| Délai « 3 mois » | Le délai de l'école est de six semaines effectives | Contradictoire | Cadre : voir `projet_et_rncp.md` §1 |
| Plateforme opérationnelle, formation MSF | Pas de client réel | Hors périmètre | `prototype_skeleton.md` §5 |

## 5. Conséquences pour la suite

1. Les décisions listées ci-dessus forment le **premier registre d'architecture** (`ADR_global.md`), établi **avant** les diagrammes définitifs.
2. Les limites ne sont pas des échecs : elles sont énoncées à la soutenance (`prototype_skeleton.md` §6).
3. En revue de code, l'équipe peut montrer **ce qu'elle a construit** et **pourquoi** chaque écart par rapport au sujet a été interprété, précisé ou limité.

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 1.0 | 04/11/2026 | Consolidation de l'analyse critique initiale (46 points, numérotation abandonnée) par thème, avec renvoi aux décisions du nouveau registre |
