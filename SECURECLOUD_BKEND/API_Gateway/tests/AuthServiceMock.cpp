#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "auth.grpc.pb.h"

class AuthServiceMock final : public auth::AuthService::Service
{
public:
    grpc::Status Login(grpc::ServerContext*,
                       const auth::LoginRequest* request,
                       auth::LoginResponse* response) override
    {
        const std::string& login = request->login();
        const std::string role = roleFor(login);

        if (role.empty() || request->password() != "azerty") {
            return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Invalid demo credentials");
        }

        response->set_access_token("demo-access-" + login);
        response->set_refresh_token("demo-refresh-" + login);
        response->set_token_type("Bearer");
        response->set_expires_in(3600);

        auto* user = response->mutable_user();
        user->set_id("demo-" + login);
        user->set_login(login);
        user->set_role(role);
        user->set_created_at("demo");

        return grpc::Status::OK;
    }

    grpc::Status ValidateToken(grpc::ServerContext*,
                               const auth::TokenRequest* request,
                               auth::TokenResponse* response) override
    {
        response->set_is_valid(
            request->token() == "demo-access-alice" ||
            request->token() == "demo-access-coord" ||
            request->token() == "demo-access-admin");
        return grpc::Status::OK;
    }

private:
    static std::string roleFor(const std::string& login)
    {
        if (login == "alice") return "STD_USER";
        if (login == "coord") return "COORD";
        if (login == "admin") return "ADMIN";
        return {};
    }
};

int main()
{
    AuthServiceMock service;
    grpc::ServerBuilder builder;
    int selectedPort = 0;
    builder.AddListeningPort("127.0.0.1:50051", grpc::InsecureServerCredentials(), &selectedPort);
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
    if (!server || selectedPort != 50051) {
        std::cerr << "Impossible de demarrer AuthServiceMock sur 127.0.0.1:50051\n";
        return 1;
    }

    std::cout << "AuthServiceMock ecoute sur 127.0.0.1:50051\n"
              << "Comptes demo: alice, coord, admin / azerty\n"
              << "Ctrl+C pour arreter.\n";
    server->Wait();
    return 0;
}
