#pragma once
#include <crow.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "MessageParser.h"
#include "MessagingClient.h"

class WssController {
public:
    explicit WssController(std::shared_ptr<MessagingClient> messagingClient)
        : messagingClient_(std::move(messagingClient)) {
    }

    // Enregistre la route WebSocket sur l'application Crow (avec ton middleware AuthFilter)
    template <typename App>
    void registerRoutes(App& app) {
        CROW_WEBSOCKET_ROUTE(app, "/ws")
            .onopen([this](crow::websocket::connection& conn) {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_.insert(&conn);
                })
            .onclose([this](crow::websocket::connection& conn, const std::string&, uint16_t) {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_.erase(&conn);
                })
            .onmessage([this](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            if (is_binary) {
                conn.send_text(R"({"status":"error","message":"Format pas encore supporte"})");
                return;
            }
            std::string response = handleIncomingText(data);
            conn.send_text(response);
                });
    }

    std::string handleIncomingText(const std::string& rawJson) {
        ParsedMessage msg = parser_.parseMessage(rawJson);

        if (!msg.isValid) {
            return R"({"status":"error","message":"JSON invalide"})";
        }

        if (msg.action != "send_message") {
            return R"({"status":"error","message":"Action inconnue"})";
        }

        if (!messagingClient_) {
            return R"({"status":"error","message":"MessagingClien pas initialiser"})";
        }

        bool sent = messagingClient_->sendMessage(msg.sender, msg.recipient, msg.content);
        if (!sent) {
            return R"({"status":"error","message":"Echec de l'envoi"})";
        }

        return R"({"status":"ok","message":"Message envoye"})";
    }

    size_t getActiveConnectionsCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connections_.size();
    }

private:
    std::shared_ptr<MessagingClient> messagingClient_;
    MessageParser parser_;
    mutable std::mutex mutex_;
    std::unordered_set<crow::websocket::connection*> connections_;
};