## 3. `WssController.h` — Contrôleur WebSocket Temps Réel

### Rôle
Point d'entrée temps réel de l'**API Gateway (Crow)** sur la route `/ws`. Il gère le cycle de vie des connexions WebSocket actives et orchestre le traitement complet de chaque message entrant : validation JSON via `MessageParser`, puis transmission gRPC au backend via `MessagingClient`.

### Architecture et mécanismes clés
* **Format *Header-Only* (`template <typename App>`) :**
  La méthode `registerRoutes(App& app)` est templatée pour s'attacher directement à l'instance applicative Crow (incluant ses middlewares comme `AuthFilter`), garantissant une intégration directe sans surcoût d'abstraction.
* **Gestion *Thread-Safe* des connexions (`std::mutex`) :**
  Les événements `.onopen` et `.onclose` maintiennent à jour l'ensemble des clients connectés (`std::unordered_set<crow::websocket::connection*>`) protégé par un `std::lock_guard<std::mutex>`, évitant toute condition de course (*data race*) sous forte charge concurrente.
* **Séparation de la logique métier (`handleIncomingText`) :**
  Le traitement du texte brut est isolé dans une méthode dédiée indépendante du socket réseau. Cela permet de tester unitairement l'intégralité du pipeline (parsing JSON + routage d'action + appel gRPC) dans GoogleTest sans ouvrir de port TCP.
* **Filtrage et réponses JSON structurées :**
  * Rejette immédiatement les trames binaires (`is_binary`).
  * Vérifie la validité structurelle via `MessageParser::parseMessage()`.
  * Route l'action (`"send_message"`) vers `MessagingClient::sendMessage()` et renvoie un accusé de réception JSON normalisé (`{"status":"ok",...}` ou `{"status":"error",...}`).

### Flux d'exécution d'un message
```text
Trame WebSocket (/ws) ??? handleIncomingText() ??? MessageParser ??? MessagingClient (gRPC) ??? Réponse JSON