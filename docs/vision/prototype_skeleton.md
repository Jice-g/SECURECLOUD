# SECURECLOUD[SKELETON] : le prototype

> **Version** 1.0 · **Date** 04/11/2026 · **Statut** brouillon à relire · **Niveau** 1 (commun)
> **Documents liés** : [projet_et_rncp.md](projet_et_rncp.md) · [../specifications/specifications_skeleton.md](../specifications/specifications_skeleton.md) · [../specifications/specifications_roadmap.md](../specifications/specifications_roadmap.md) · [../decisions/ADR_global.md](../decisions/ADR_global.md)

---

## 1. Pourquoi une version SKELETON

Le cahier des charges décrit un produit industriel : 45 000 utilisateurs, 10 000 messages par seconde, 99,9 % de disponibilité, cinq microservices, un client multiplateforme et une chaîne de déploiement complète. Un projet d'école de six semaines, à trois, ne permet pas de construire et **de prouver** tout cela.

> **Règle de cadrage fondamentale.** Nous ne cherchons pas à implémenter toute la vision industrielle à son niveau de production. Nous construisons un **prototype techniquement cohérent**, qui démontre les propriétés essentielles de l'architecture.

SECURECLOUD[SKELETON] est à la fois :

- un **prototype** ;
- un **MVP** (*Minimum Viable Product*) dans les délais impartis ;
- une **pièce d'examen** conforme aux objectifs pédagogiques et aux compétences du RNCP ;
- un **dépôt public** destiné aux recruteurs.

Il est accompagné d'une **roadmap** (`specifications_roadmap.md`) qui décrit les étapes pour atteindre une V1.

Enjeux majeurs retenus : **la scalabilité de l'architecture, la qualité des tests dans une pipeline CI/CD, et la culture DevOps**.

## 2. Classement de chaque exigence : SK / RM / DS

Chaque exigence du cahier des charges reçoit une décision, identifiée dans le registre (`ADR_global.md`).

| Statut | Signification |
|---|---|
| **SK** | Retenu et implémenté dans SKELETON ; démontré à la soutenance |
| **SK simplifié** | Retenu, dans une version volontairement réduite (précisée dans la spécification) |
| **RM** | Reporté en roadmap (V1 ou V2) ; hors des six semaines, documenté comme suite prévue |
| **DS** | Écarté (*discard*) : non démontrable ou hors capacité du projet. Conservé comme **limite assumée et documentée**, jamais contourné par une affirmation non vérifiée |

Deux nuances :

- Une exigence peut être scindée : par exemple « 10 000 msg/s » est **SK pour la méthode de mesure** et **DS pour la valeur**.
- Un statut est une décision d'équipe. Le cahier des charges précise que toute modification doit être validée par l'équipe **et** le client : la validation par l'encadrement pédagogique est une étape de la roadmap du projet.

## 3. Ce que SKELETON démontre

| Propriété d'architecture | Comment elle est démontrée |
|---|---|
| Application **organisée en couches**, domaine indépendant de la technique | Domaine C++ pur testable sans Qt ni réseau ; ports et adaptateurs (voir l'architecture du Client) |
| **Le serveur ne voit jamais le contenu** des messages | Chiffrement côté Client avant envoi ; test montrant que le serveur ne manipule que des enveloppes opaques |
| Communication **REST/WebSocket** à l'extérieur, **gRPC** à l'intérieur | Client → Gateway → services, contrats écrits |
| **Authentification** par jetons courts, révocation fonctionnelle | Scénarios E2E de login, test de révocation |
| **Chaîne CI/CD** complète sur un environnement de démonstration | Pipeline build → test → package → deploy → verify, mesure du délai |
| **Qualité des tests** | Tests unitaires, intégration et E2E ; couverture définie par cas d'usage (TEST-01) |
| Fonctionnement en **réseau dégradé** | Simulation d'un lien à 256 kbps (throttling) ; file d'attente locale et reprise |

## 4. Périmètre fonctionnel

Les cas d'utilisation sont décrits dans `use_cases/` (lot suivant). Leur statut :

| UC | Cas d'utilisation | Statut | Note |
|---|---|---|---|
| UC-01 | Se connecter | **SK** | Première tranche livrée de bout en bout |
| UC-02 | Envoyer un message individuel | **SK** | |
| UC-03 | Envoyer un message dans un groupe | **RM** | Le modèle anticipe les groupes ; l'envoi est refusé explicitement |
| UC-04 | Joindre une pièce jointe | **SK simplifié** | Après décision FILE-06 (nom de fichier) |
| UC-05 | Consulter ses conversations | **SK** | |
| UC-06 | Recevoir un message | **SK** | |
| UC-07 | Gérer les groupes de discussion | **RM** | |
| UC-08 | Télécharger une pièce jointe | **SK simplifié** | |
| UC-09 | Se déconnecter | **SK** | Locale d'abord ; révocation serveur ensuite |
| UC-10 | Gérer les alertes prioritaires | **SK** | Cœur fonctionnel retenu (MSG-05) |
| UC-11 | Recevoir et acquitter une alerte | **SK** | |
| UC-12 | Gérer les groupes d'alerte | **RM** | En SKELETON les groupes d'alerte sont **préchargés** (proposé) |

**Ordre de livraison : à arbitrer en équipe.** La feuille de route du Client place les pièces jointes avant les alertes, alors que la spécification désigne les alertes comme cœur fonctionnel. L'ordre doit être fixé avant la semaine prochaine ; en cas de retard, le dernier bloc livré est le premier sacrifié, jamais les tests.

## 5. Livrables SKELETON

| Livrable du cahier des charges | Décision SKELETON |
|---|---|
| Microservices C++ conteneurisés | **SK** : 4 services métier (auth, messaging, files, audit) + API Gateway |
| Client Qt multiplateforme | **SK** Windows et Linux ; macOS en *best effort* |
| Pipeline CI/CD complète | **SK**, sur environnement de démonstration |
| Documentation technique complète | **SK** |
| Tests automatisés « couverture 100 % » | **SK redéfini** : voir TEST-01 |
| Plateforme opérationnelle pour MSF | **DS** : remplacée par un *démonstrateur fonctionnel validé en environnement pédagogique* |
| Formation des équipes MSF | **DS** : pas de client réel |
| Procédures de déploiement, plan de maintenance | **SK**, pour l'environnement de démonstration |

## 6. Limites assumées

Ces limites seront énoncées telles quelles à la soutenance. Les écrire est une qualité, pas un aveu.

- « Inviolable » n'est pas démontré ; un **modèle de menace** et des propriétés vérifiables le remplacent (SEC-01).
- Le débit de 10 000 msg/s n'est pas prouvé ; on publie la valeur **mesurée** (PERF-01).
- Les 99,9 % de disponibilité sont un **objectif d'architecture cible**, pas une mesure (OPS-01).
- 500 Mo en 3 minutes et 256 kbps ne sont **jamais** présentés comme simultanés (PERF-03).
- 45 000 utilisateurs : jeu de données de quelques centaines (AUTH-04).
- Aucun audit de sécurité externe (TEST-02) : une revue cryptographique interne documentée.
- Les conditions physiques (45 °C, poussière, matériel vieillissant) ne sont pas testables (QT-03).
- Stockage des clés du Client : fichier local chiffré, sans coffre natif de l'OS ; limite de sécurité assumée pour la démonstration.
- PostgreSQL en instance unique, pas de réplication ; Docker Compose mono-nœud.

## 7. Critères de réussite de SKELETON

SKELETON est réussi si, à la soutenance :

1. la pile démarre avec **une commande** (`docker compose up`) ;
2. les scénarios E2E de login passent (réussite, mauvais mot de passe, service arrêté, Gateway arrêtée, identifiant inconnu) ;
3. deux Clients échangent un message dont le **contenu reste illisible pour le serveur** ;
4. la pipeline CI/CD s'exécute de bout en bout et son délai est mesuré ;
5. chaque cas d'usage retenu est couvert par au moins un test d'intégration ;
6. chaque écart par rapport au cahier des charges est **expliqué et tracé** (cahier → analyse → décision → spécification → test).

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 1.0 | 04/11/2026 | Première rédaction « au propre ». Corrige : « 5 microservices » devient « 4 services métier + Gateway » ; `deploy-service` classé RM |
