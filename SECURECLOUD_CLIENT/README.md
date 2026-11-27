# SecureCloud — Client

> Application cliente de bureau de **SecureCloud**, plateforme de communications professionnelles sécurisées.
> Fait partie du prototype d'école **SECURECLOUD[SKELETON]** (voir le dépôt global).

Le client est une application native en **C++20 et Qt 6 (Widgets)**, organisée en couches pour que les règles métier restent indépendantes de l'interface, du réseau et du stockage.

## Fonctionnalités visées

| Fonction | Description |
|---|---|
| Connexion / déconnexion | Authentification auprès du serveur, gestion de session |
| Messagerie 1-to-1 | Envoi et réception, priorités (normal, haute, critique), historique |
| Pièces jointes | Envoi et téléchargement de fichiers, transfert chiffré par blocs |
| Alertes | Émission vers un groupe d'alerte, réception, acquittement |
| Recherche locale | Recherche dans l'historique, côté client uniquement |
| Hors-ligne | Les messages sont mis en file locale puis renvoyés au retour du réseau |

Le contenu des messages est chiffré **côté client** avant l'envoi : le serveur ne peut pas le lire.

## Architecture

```
            ┌───────────────────────────┐
            │            UI             │   Qt Widgets, aucune règle métier
            └─────────────┬─────────────┘
                          ▼   DTO
            ┌───────────────────────────┐
            │        Application        │   cas d'usage, session, ports
            └──────┬─────────────▲──────┘
                   ▼             │ implémente les ports
            ┌─────────────┐  ┌───────────────────┐
            │   Domain    │  │  Infrastructure   │   réseau, crypto, stockage
            └─────────────┘  └───────────────────┘
                     Shared : types et utilitaires communs
```

| Couche | Rôle |
|---|---|
| **Domain** | Entités (`User`, `Message`, `Conversation`...), objets-valeur, interfaces de repositories, règles métier pures. C++ standard uniquement : aucune dépendance à Qt, au réseau ou à une base de données. |
| **Application** | Un cas d'usage par classe (`LoginUserUseCase`, `SendMessageUseCase`...), gestion de session, DTO, et **ports** (`ITransport`, `ICryptoProvider`, `ILocalStorage`, `ISessionGateway`). |
| **Infrastructure** | Implémentations concrètes des ports : WebSocket/HTTPS, libsodium, SQLite. |
| **UI** | Fenêtres Qt. Communique uniquement avec l'Application, par DTO. |
| **Shared** | Constantes, types d'erreur et utilitaires transverses. |

Règle de dépendance : le Domain ne connaît aucune autre couche. L'Infrastructure est remplaçable (inversion de dépendance), ce qui permet de passer d'un mock à une implémentation réelle sans toucher au reste.

## Choix techniques

| Élément | Choix |
|---|---|
| Langage | C++20 |
| Interface | Qt 6.8.3 LTS, Qt Widgets |
| Build | CMake, Ninja |
| Compilateur de référence | MSVC 2022 64-bit (Windows) ; Linux pris en charge |
| Réseau | HTTPS et WebSocket vers l'API Gateway |
| Cryptographie | libsodium (aucune primitive écrite « maison ») |
| Stockage local | En mémoire pour les premières itérations, SQLite ensuite |
| Tests | Tests unitaires du domaine, exécutés avec CTest |

## Arborescence

```
SecureCloud-Client/
├── CMakeLists.txt
├── docs/            spécifications, ADR, domaine, architecture, tests
├── assets/          icônes, images, feuilles de style
├── tests/           tests du domaine, de l'application, d'intégration
└── src/
    ├── main.cpp
    ├── domain/          modèle, repositories (interfaces), services
    ├── application/     cas d'usage, DTO, ports, session
    ├── infrastructure/  implémentations des ports
    ├── shared/          types et utilitaires communs
    └── ui/              fenêtres Qt
```

## Compiler et tester

**Prérequis** : Qt 6.8.3, CMake 3.21 ou plus, Ninja, un compilateur C++20 (MSVC 2022 ou GCC/Clang récent).

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=<chemin vers le kit Qt>
cmake --build build
ctest --test-dir build --output-on-failure
```

Le modèle du domaine ne dépend pas de Qt : ses tests peuvent aussi être compilés avec un simple compilateur C++20.

## État d'avancement

| Élément | État |
|---|---|
| Modèle du domaine (entités, objets-valeur) et tests unitaires | Réalisé |
| Interfaces de repositories et services de domaine | En cours |
| Connexion (cas d'usage, session, contrat avec la Gateway) | À venir |
| Messagerie, pièces jointes, alertes | À venir |
| Infrastructure réelle (réseau, crypto, SQLite) | À venir |
| Interface Qt | Squelette |

## Méthode de travail

Le code découle de spécifications validées : **exigence → analyse → décision → spécification → implémentation → test**. Chaque cas d'usage est documenté (séquences, algorithmes) avant d'être codé, et le domaine est testé isolément.

## Documentation

Le dossier `docs/` regroupe la vision du projet, les spécifications et leurs décisions (ADR), le modèle du domaine, l'architecture et la stratégie de tests.

