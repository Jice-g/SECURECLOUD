#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <crow.h>
#include "AuthClient.h"

class AuthController {
public:
    explicit AuthController(AuthClient& authClient)
        : authClient_(authClient) {
    }

    template <typename App>
    void registerRoutes(App& app) {
        CROW_ROUTE(app, "/auth/login").methods(crow::HTTPMethod::POST)
            ([this](const crow::request& req) {
            return handleLogin(req);
                });
    }

    crow::response handleLogin(const crow::request& req) {
        // Limite octets
        constexpr size_t kMaxPayloadBytes = 2048;
        if (req.body.size() > kMaxPayloadBytes) {
            return makeErrorResponse(413, "Trop gros", "Depasse 2 Ko");
        }

        // validation du JSON
        auto bodyJson = crow::json::load(req.body);
        if (!bodyJson || bodyJson.t() != crow::json::type::Object) {
            return makeErrorResponse(400, "JSON", "Invalide.");
        }

        if (!bodyJson.has("login") || bodyJson["login"].t() != crow::json::type::String ||
            !bodyJson.has("password") || bodyJson["password"].t() != crow::json::type::String) {
            return makeErrorResponse(400, "Manque", "login et password obligatoires");
        }

        const std::string login = bodyJson["login"].s();
        const std::string password = bodyJson["password"].s();

        if (login.empty() || password.empty()) {
            return makeErrorResponse(400, "EMPTY_FIELDS", "Le login et le mot de passe ne peuvent pas etre vide");
        }

        std::string mfaCode;
        if (bodyJson.has("mfa_code") && bodyJson["mfa_code"].t() == crow::json::type::String) {
            mfaCode = bodyJson["mfa_code"].s();
        }

        // 5 tentative 5 min
        const std::string clientIp = req.remote_ip_address.empty() ? "unknown" : req.remote_ip_address;
        const std::string rateKey = clientIp + ":" + login;

        int retryAfterSeconds = 0;
        if (isRateLimited(rateKey, retryAfterSeconds)) {
            crow::response res = makeErrorResponse(429, "TOO_MANY_REQUESTS", "Trop de tentatives de connexion");
            res.add_header("Retry-After", std::to_string(retryAfterSeconds));
            return res;
        }

        // Appel vers auth-service
        LoginResult result = authClient_.login(login, password, mfaCode);

        // Gestion des erreurs gRPC -> HTTP
        if (!result.status.ok()) {
            recordFailedAttempt(rateKey);
            return mapGrpcErrorToHttp(result.status);
        }

        // Reset
        resetAttempts(rateKey);

        // Reponse 200 OK
        crow::json::wvalue okJson;
        okJson["access_token"] = result.access_token;
        okJson["refresh_token"] = result.refresh_token;
        okJson["token_type"] = result.token_type.empty() ? "Bearer" : result.token_type;
        okJson["expires_in"] = result.expires_in;
        okJson["user"]["id"] = result.user_id;
        okJson["user"]["login"] = result.user_login;
        okJson["user"]["role"] = result.user_role;
        okJson["user"]["created_at"] = result.created_at;

        crow::response res(200, okJson.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    }

private:
    AuthClient& authClient_;
    std::mutex rateMutex_;
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> attempts_;

    static constexpr size_t kMaxAttempts = 5;
    static constexpr std::chrono::seconds kWindowDuration{ 300 }; // 5 minutes

    bool isRateLimited(const std::string& key, int& retryAfterSeconds) {
        std::lock_guard<std::mutex> lock(rateMutex_);
        const auto now = std::chrono::steady_clock::now();
        auto& history = attempts_[key];

        while (!history.empty() && (now - history.front()) >= kWindowDuration) {
            history.pop_front();
        }

        if (history.size() >= kMaxAttempts) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - history.front());
            retryAfterSeconds = static_cast<int>((kWindowDuration - elapsed).count());
            if (retryAfterSeconds < 1) retryAfterSeconds = 1;
            return true;
        }

        return false;
    }

    void recordFailedAttempt(const std::string& key) {
        std::lock_guard<std::mutex> lock(rateMutex_);
        attempts_[key].push_back(std::chrono::steady_clock::now());
    }

    void resetAttempts(const std::string& key) {
        std::lock_guard<std::mutex> lock(rateMutex_);
        attempts_.erase(key);
    }

    static crow::response makeErrorResponse(int httpCode, const std::string& errorCode, const std::string& message) {
        crow::json::wvalue errJson;
        errJson["error"] = errorCode;
        errJson["message"] = message;

        crow::response res(httpCode, errJson.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    }

    static crow::response mapGrpcErrorToHttp(const grpc::Status& status) {
        switch (status.error_code()) {
        case grpc::StatusCode::UNAUTHENTICATED:
            return makeErrorResponse(401, "INVALID_CREDENTIALS", "Identifiant ou mot de passe incorrect.");
        case grpc::StatusCode::PERMISSION_DENIED:
            return makeErrorResponse(403, "ACCOUNT_DISABLED", "Compte desactive ou acces refuse.");
        case grpc::StatusCode::INVALID_ARGUMENT:
            return makeErrorResponse(400, "INVALID_ARGUMENT", "Parametres de connexion invalides.");
        case grpc::StatusCode::DEADLINE_EXCEEDED:
            return makeErrorResponse(504, "GATEWAY_TIMEOUT", "Le service d'authentification n'a pas repondu a temps.");
        case grpc::StatusCode::UNAVAILABLE:
            return makeErrorResponse(503, "SERVICE_UNAVAILABLE", "Le service d'authentification est indisponible.");
        default:
            return makeErrorResponse(502, "BAD_GATEWAY", "Erreur interne du service d'authentification.");
        }
    }
};