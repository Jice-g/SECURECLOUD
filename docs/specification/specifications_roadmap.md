# Roadmap SecureCloud : spécifications reportées (V1 et V2)

> **Version** 1.0 · **Date** 04/11/2026 · **Statut** brouillon à relire · **Niveau** 1 (commun)
> **Documents liés** : [specifications_skeleton.md](specifications_skeleton.md) · [../decisions/ADR_global.md](../decisions/ADR_global.md)

Ce document recense tout ce qui a été **classé RM** (ou DS avec possibilité de retour) dans SKELETON. Il décrit la suite prévue ; **rien ici n'est promis pour la soutenance**.

**Repères (indicatifs).**

- **V1** : produit fonctionnellement complet sur un environnement de démonstration durci. Les fonctions écartées de SKELETON pour manque de temps y reviennent.
- **V2** : passage à l'échelle et à la production : haute disponibilité, volumes réels, audit externe.

Chaque ligne indique la **spécification d'origine**, la **raison du report**, le **prérequis** et le **repère**.

---

## 1. Messagerie

| ID | Fonction | Origine | Raison du report | Prérequis | Repère |
|---|---|---|---|---|---|
| RM-01 | Groupes de discussion (jusqu'à 100 participants) | F1.1, MSG-04 | Diffusion à N, permissions dynamiques, clés de groupe, volume de tests | Spécification de gestion des clés de groupe dans `Crypto.md` ; file d'attente par destinataire | V1 |
| RM-02 | Notifications de mention | F1.4 | Dépend des groupes | RM-01 | V1 |
| RM-03 | Présence `AWAY` | F1.4, MSG-02 | Arbitrage du 16/09/2026 ; troisième valeur du même type, pas un nouveau modèle | Règles de visibilité par permission | V1 |
| RM-04 | Threading multi-niveaux | F1.5, MSG-01 | SKELETON : simple réponse | Modèle de thread arrêté | V1 |
| RM-05 | Gestion des groupes d'alerte (création, membres) | UC-12, MSG-05 | SKELETON : groupes préchargés | Permissions d'administration | V1 |
| RM-06 | Relance après non-acquittement, clôture d'alerte | F1.9 | Règle métier avec délai non tranchée | Décision sur le délai et la nature de la relance | V1 |
| RM-07 | Priorité garantie par l'infrastructure (préemption, QoS) | F1.8, MSG-03 | La priorité SKELETON est logique | Étude de qualité de service | V2 |

## 2. Fichiers

| ID | Fonction | Origine | Raison du report | Prérequis | Repère |
|---|---|---|---|---|---|
| RM-08 | Reprise de transfert avancée (nouvel essai exponentiel, délais paramétrables) | F2.2, FILE-03 | SKELETON : relance simple du bloc | Protocole de transfert stabilisé | V1 |
| RM-09 | Déduplication compatible avec le chiffrement de bout en bout | F2.5, FILE-02 | Incompatible avec un modèle strict ; augmente fortement la complexité | Mécanisme de déduplication spécifié et analysé (étude de sécurité) | V2, sous réserve |
| RM-10 | Système de fichiers virtuel chiffré | F2.4, FILE-05 | Hors proportion | Retour d'expérience sur le stockage fichier par fichier | V2 |
| RM-11 | Sauvegarde sans Internet vers un support externe ou un serveur local | F2.7, FILE-01 | SKELETON : stockage local et synchronisation différée | Choix du support, du protocole de synchronisation | V2 |

## 3. Authentification et sécurité

| ID | Fonction | Origine | Raison du report | Prérequis | Repère |
|---|---|---|---|---|---|
| RM-12 | MFA complet (TOTP généralisé, WebAuthn) | F3.1, AUTH-02 | SKELETON : TOTP ou MFA simulé | Choix des facteurs ; gestion de l'enrôlement | V1 |
| RM-13 | Persistance chiffrée du jeton de rafraîchissement et rafraîchissement automatique côté Client | F3.2 | Première tranche : jetons en mémoire | Stockage sécurisé du Client | V1 |
| RM-14 | Récupération d'identifiant et de mot de passe | UC-01 | Hors première tranche | Procédure de réinitialisation sécurisée | V1 |
| RM-15 | Matrice de permissions complète (User/Role/Permission/Resource/Action/Scope) | F3.4, AUTH-03 | SKELETON : RBAC à 3 rôles | Matrice rédigée ; décision sur l'héritage de rôles | V1 |
| RM-16 | Révocation avec garantie de délai | F3.5, AUTH-05 | Pas de SLA en SKELETON | Mesure sous charge ; cache à invalidation | V1 |
| RM-17 | Intégration aux coffres natifs des OS pour les clés du Client | Choix de stockage des clés (registre du Client) | SKELETON : fichier chiffré local | Choix par OS (Windows, Linux, macOS) | V1 |
| RM-18 | Journal d'audit à inaltérabilité cryptographique (chaînage) | F5.4, AUD-01 | SKELETON : journal en ajout seul | Conception du chaînage | V1 |
| RM-19 | Audit de sécurité externe | N6, TEST-02 | Hors capacité et budget | Budget, prestataire | V2 |

## 4. Architecture, déploiement et exploitation

| ID | Fonction | Origine | Raison du report | Prérequis | Repère |
|---|---|---|---|---|---|
| RM-20 | `deploy-service` (service d'orchestration) | A6, ARC-06 | Rôle tenu par CI/CD et Compose | Besoin pédagogique ou produit démontré | V1 ou V2 |
| RM-21 | Haute disponibilité : réplication PostgreSQL, instances multiples, répartition de charge | ARC-04, ARC-05, OPS-01 | Compose mono-nœud | Orchestrateur adapté | V2 |
| RM-22 | Capacité à 45 000 utilisateurs réels | F3.6, AUTH-04 | Volume non démontrable en école | RM-21, tests de charge | V2 |
| RM-23 | Mesure de disponibilité sur longue période (99,9 %) | OPS-01 | Non mesurable avec le prototype | RM-21 ; supervision | V2 |
| RM-24 | Déploiement signé en production : signature des artefacts, gestion de secrets, approbation, retour arrière automatisé | CI-01 | SKELETON : démonstrateur | Politique de livraison | V2 |
| RM-25 | Benchmarks automatisés en CI (démarrage, mémoire, latence) | QT-02, PERF-02 | SKELETON : mesures ponctuelles | Environnement de mesure stable | V1 |
| RM-26 | Build macOS officiel | F4.1, QT-01 | Pas de machine de test | Poste macOS ou CI macOS | V1 |

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 1.0 | 04/11/2026 | Première rédaction ; remplace la mention dispersée « RM » de la spécification v1 |
