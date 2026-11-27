# SecureCloud

> Plateforme de communications professionnelles sécurisées pour organisations humanitaires.
> Prototype d'école : **SECURECLOUD[SKELETON]**.

Projet réalisé par une équipe de trois dans le cadre du Bachelor IT « Développeur logiciel / DevOps » (B3, La Plateforme), en six semaines effectives. Il s'appuie sur le référentiel du titre professionnel *Concepteur développeur d'applications* (RNCP 37873). Cas d'usage de référence : Médecins Sans Frontières (client fictif).

## Vision

Offrir aux équipes de terrain une messagerie et un partage de fichiers où **le serveur ne voit jamais le contenu** des échanges :

- messagerie instantanée chiffrée de bout en bout ;
- partage de fichiers sécurisé, avec journal d'audit des actions (jamais du contenu) ;
- alertes prioritaires diffusées à un groupe, avec suivi d'acquittement ;
- infrastructure auto-hébergée et déploiement automatisé.

## Périmètre de la version SKELETON

Le cahier des charges décrit une ambition industrielle. SKELETON en retient le cœur, démontrable en six semaines :

| Domaine | Réalisé dans le prototype |
|---|---|
| Authentification | Connexion, jetons d'accès courts et rafraîchissement, révocation, rôles (utilisateur, coordinateur, administrateur) |
| Messagerie | Conversations 1-to-1, priorités (normal, haute, critique), historique et recherche locale |
| Alertes | Émission vers un groupe d'alerte, suivi des acquittements |
| Fichiers | Transfert chiffré par blocs, reprise simple après coupure, sauvegarde locale hors-ligne puis synchronisation |
| Audit | Journal des événements de sécurité et d'administration |
| Déploiement | Pipeline CI/CD (build, tests, images Docker, déploiement de démonstration, vérification) |

Prévu pour la suite : groupes de discussion, permissions plus fines, haute disponibilité de la base de données.

## Architecture

```
  Client Qt (Windows / Linux)
        │   HTTPS  +  WebSocket sécurisé
        ▼
   API Gateway   (validation des jetons, routage)
        │   gRPC
        ├── auth-service        authentification, sessions, rôles
        ├── messaging-service   messages, alertes, présence
        ├── files-service       transfert de fichiers par blocs
        └── audit-service       journal d'audit (asynchrone)
                 │
                 ▼
            PostgreSQL
```

Le chiffrement de bout en bout se fait **dans le client** : les services backend ne manipulent que des données opaques et les métadonnées nécessaires au routage.

## Choix techniques

| Élément | Choix |
|---|---|
| Services | C++, communication interne en gRPC |
| Client | C++20, Qt 6 (Widgets), architecture en couches |
| Cryptographie | Bibliothèques éprouvées (libsodium, OpenSSL). Aucune primitive écrite « maison » |
| Authentification | Jetons JWT via une bibliothèque standard, hachage de mots de passe standard |
| Données | PostgreSQL |
| Déploiement | Docker et Docker Compose, pipeline CI/CD |

## Démarche

Chaque exigence suit la chaîne **exigence → analyse → décision → spécification → implémentation → test → preuve**. Les décisions sont consignées sous forme d'ADR et chaque spécification est classée :

- **SK** : réalisée dans le prototype ;
- **RM** : reportée en roadmap ;
- **DS** : écartée, avec la limite documentée.

Les exigences absolues du cahier des charges (« inviolable », « zéro panne ») sont remplacées par des propriétés vérifiables : modèle de menace, objectifs mesurables, benchmarks reproductibles.

## Dépôts et documentation

| Élément | Contenu |
|---|---|
| `SecureCloud-Client` | Client Qt : domaine, application, infrastructure, interface, tests |
| Dépôt(s) serveur | API Gateway et microservices, Docker Compose, pipeline CI/CD |
| `docs/` de chaque dépôt | Spécifications, ADR, modèle du domaine, architecture, stratégie de tests |

## Limites assumées

- Aucun audit de sécurité externe n'a été réalisé : la revue cryptographique est interne et documentée.
- Les objectifs de performance (débit, latence) sont mesurés en environnement de démonstration et publiés tels quels.
- Les conditions physiques extrêmes décrites dans le sujet (chaleur, poussière) ne sont pas reproductibles ; seul le réseau dégradé est simulé.

## Équipe

Abdelrahmane Amiri · Sana Bouchal · Jean-Christophe Gerace


