# API Gateway

## Structure

- `include/` : headers
- `src/` : code c++
- `proto/` : def des services proto
- `tests/` : tests unitaires
- `docs/` : documentation

## Dépendances

CMake, un compilateur C++17, ainsi que Crow, gRPC et Protobuf déjà installés.

## Compilation sans installation automatique

Indiquer à CMake le préfixe où les fichiers `*Config.cmake` des dépendances sont déjà installés. Utiliser un nouveau dossier de build et le même toolset MSVC que celui utilisé pour compiler ces bibliothèques :

```powershell
cmake -S . -B build-no-vcpkg -DCMAKE_PREFIX_PATH="C:\chemin\vers\dependances" -DCMAKE_BUILD_TYPE=Release
cmake --build build-no-vcpkg
```

L'exécutable est dans le dossier `build-no-vcpkg`. Les DLL présentes dans le préfixe indiqué sont copiées à côté de l'exécutable pour permettre son lancement.
