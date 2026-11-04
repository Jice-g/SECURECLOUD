# SecureCloud : vision produit

> **Version** 1.0 · **Date** 04/11/2026 · **Statut** brouillon à relire · **Niveau** 1 (commun)
> **Documents liés** : [prototype_skeleton.md](prototype_skeleton.md) · [analyse_cahier_des_charges.md](analyse_cahier_des_charges.md) · [../decisions/ADR_global.md](../decisions/ADR_global.md)

Ce document est le **« pourquoi »** du produit. Il conserve volontairement l'ambition du cahier des charges. **Rien de ce qui est écrit ici n'est une exigence technique testable** : les exigences vérifiables sont dans `specifications/`.

---

## 1. La vision

> **Créer la référence mondiale en communication sécurisée pour les entreprises, journalistes, activistes et citoyens soucieux de leur vie privée.**

Les organisations qui manipulent des informations sensibles utilisent aujourd'hui des outils grand public (WhatsApp, Gmail, Telegram, Slack) dont les garanties ne correspondent pas à leurs risques. SecureCloud propose une messagerie et un partage de fichiers chiffrés de bout en bout, avec une **expérience utilisateur professionnelle**, sur une infrastructure que l'organisation héberge elle-même.

## 2. Le client de référence : MSF

| Élément | Source : cahier des charges |
|---|---|
| Taille | 45 000 volontaires dans 70 pays, certains en zones de conflit |
| Pratique actuelle | WhatsApp et Gmail pour coordonner les missions |
| Risque | Surveillance, compromission de la sécurité des équipes terrain |
| Contexte d'usage | Connexions satellite faibles (256 kbps), chaleur, poussière, matériel vieillissant |

MSF est un **client fictif** : il fournit un scénario réaliste qui guide les choix, pas un contrat réel.

## 3. Segments visés

| Segment | Besoin principal |
|---|---|
| Journalisme d'investigation | Protéger les sources et les documents sensibles |
| ONG internationales | Communiquer en zones hostiles |
| Entreprises sensibles (finance, santé, défense) | Confidentialité et traçabilité |
| Administrations | Souveraineté numérique et conformité |

## 4. Positionnement

| Concurrent | Point fort | Limite selon le cahier des charges |
|---|---|---|
| Signal | Chiffrement de bout en bout éprouvé | Interface peu professionnelle (pas d'audit, pas de gestion centralisée) |
| Slack Enterprise | Interface moderne | Le fournisseur peut lire les messages |
| Microsoft Teams | Intégration en entreprise | Surveillance possible par des tiers |

> Ces appréciations viennent du cahier des charges ; l'équipe ne les a pas vérifiées. Le tableau est **conservé comme positionnement produit**, sans valeur d'engagement technique pour SKELETON.

**Avantage visé :** une sécurité forte *et* une ergonomie professionnelle, avec un serveur qui **ne voit jamais le contenu** des communications.

## 5. Ce que la promesse veut dire exactement

Le cahier des charges emploie des formules absolues (« inviolable », « impossibilité d'interception », « le serveur ne voit JAMAIS »). Pour qu'elles restent honnêtes, voici leur traduction vérifiable.

| Formule du cahier | Ce que SecureCloud s'engage à vérifier |
|---|---|
| « Le serveur ne voit jamais le contenu » | Le **contenu** des messages et des fichiers est chiffré par le Client avant envoi et n'est jamais déchiffrable par un service du serveur |
| … | Le serveur voit les **métadonnées nécessaires au routage** : expéditeur, conversation, destinataires, priorité, horodatage, taille |
| … | Le serveur **connaît l'identité** de l'utilisateur (login, rôle) : il en a besoin pour l'authentifier |
| « Impossibilité d'interception » | Un attaquant réseau (passif ou actif) ne lit ni ne modifie le contenu sans être détecté |
| « Résistance aux attaques étatiques » | **Non traduisible en propriété testable** par le projet ; documenté comme objectif de vision |
| « Inviolable » | Idem : remplacé par un modèle de menace avec des limites explicites |

## 6. Modèle de menace préliminaire

Base de travail pour la spécification cryptographique (`Crypto.md`, à écrire). Il sera complété avant le développement du module de chiffrement.

| Scénario | Ce que SecureCloud protège | Limite assumée |
|---|---|---|
| Interception passive du réseau | Confidentialité (TLS entre Client et Gateway, contenu chiffré de bout en bout) | Métadonnées de trafic visibles (adresses IP, taille, rythme) |
| Interception active (homme du milieu) | Certificat **et** nom vérifiés ; aucune désactivation de la vérification | — |
| Serveur compromis, sans les clés | Le contenu reste illisible | Un serveur compromis voit les métadonnées et peut refuser le service |
| Vol de session (jeton volé) | Jetons courts, révocation, rate limiting | Fenêtre résiduelle jusqu'à propagation de la révocation |
| Compromission d'un poste Client | **Hors garantie** du prototype | Les clés locales de SKELETON n'ont pas la protection d'un coffre système |
| Compromission d'une clé | Rotation et révocation à spécifier (`Crypto.md`) | Sans protocole de groupe, pas de secret persistant en V1 |

## 7. Le contexte d'usage comme source d'exigences

Le contexte extrême de MSF n'est pas décoratif : il pousse à des choix concrets.

- **Satellite géostationnaire :** environ 600 ms d'aller-retour. Un login complet coûte de l'ordre de 2 secondes. C'est la **latence**, pas le débit de 256 kbps, qui est le facteur limitant pour de petits messages.
- **Débit faible :** un fichier volumineux ne peut pas passer vite. Il faut du découpage, de la reprise et une file d'attente (PERF-03, FILE-03).
- **Pas de réseau du tout :** le Client doit stocker localement, chiffré, puis synchroniser au retour du réseau (FILE-01).
- **Matériel vieillissant :** démarrage rapide, faible empreinte mémoire (QT-02) ; choix d'un client natif plutôt que web.

## 8. Cinq scénarios de crise MSF

Ces scénarios, tirés du cahier des charges, servent de **cas de test cibles**. Leur traitement réaliste est précisé dans `specifications_skeleton.md` (§ Tests).

| # | Scénario | Idée directrice |
|---|---|---|
| 1 | Mission en Afghanistan, login sur liaison satellite instable | Résister à la latence et aux coupures |
| 2 | Évacuation d'urgence, révocation immédiate d'un accès | Une session révoquée ne redevient jamais active |
| 3 | Scanner d'urgence : upload d'imagerie de 500 Mo | Transfert découpé et vérifié |
| 4 | Isolement réseau total, sauvegarde terrain | Stockage local chiffré, synchronisation différée |
| 5 | Urgence chirurgicale, message de priorité maximale | La priorité est **logique** ; ce n'est pas une garantie de livraison instantanée |

---

## Journal des changements

| Version | Date | Changement |
|---|---|---|
| 1.0 | 04/11/2026 | Première rédaction « au propre » ; ajout de la traduction vérifiable de la promesse et du modèle de menace préliminaire |
