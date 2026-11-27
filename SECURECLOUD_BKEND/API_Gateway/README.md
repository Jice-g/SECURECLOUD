# API Gateway

## Structure

- `include/` : headers
- `src/` : code c++
- `proto/` : def des services proto
- `tests/` : tests unitaires
- `docs/` : documentation

## Dépendances

CMake,  Crow, gRPC et Protobuf.

## MOC auth pour la demo

`AuthServiceMock` est le serveur gRPC de démo. Il écoute  sur `127.0.0.1:50051` et accepte les comptes  `alice`, `coord` ou `admin`, avec le mot de passe `azerty`.

## HTTPS local avec Caddy

Pour un test sur le même ordinateur, Caddy termine TLS sur `localhost:8443` et transmet les requêtes à Crow sur `127.0.0.1:8085`.

Le certificat est signé par l'autorité locale de Caddy. Dans le terminal utilisé pour lancer le client.

Ce certificat est destiné au test local sur `localhost` uniquement. Pour un autre ordinateur, il faut un nom d'hôte accessible depuis le client et un certificat qui couvre ce nom.
