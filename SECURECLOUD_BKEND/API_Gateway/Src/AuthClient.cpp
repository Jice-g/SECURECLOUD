#include "AuthClient.h"
#include <chrono>

AuthClient::AuthClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(auth::AuthService::NewStub(channel)) {
}

bool AuthClient::validateToken(const std::string& token) {
    
    auth::TokenRequest request;
    request.set_token(token);

    auth::TokenResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->ValidateToken(&context, request, &response);

    
    if (status.ok()) {
        return response.is_valid();
    }

    return false;
}

LoginResult AuthClient::login(const std::string& loginStr, const std::string& password, const std::string& mfaCode) {
    LoginResult result;

    if (!stub_) {
        result.status = grpc::Status(grpc::StatusCode::INTERNAL, "Client gRPC");
        return result;
    }

    auth::LoginRequest request;
    request.set_login(loginStr);
    request.set_password(password);
    if (!mfaCode.empty()) {
        request.set_mfa_code(mfaCode);
    }

    auth::LoginResponse response;
    grpc::ClientContext context;


    context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(5000));

    result.status = stub_->Login(&context, request, &response);

    if (result.status.ok()) {
        result.access_token = response.access_token();
        result.refresh_token = response.refresh_token();
        result.token_type = response.token_type();
        result.expires_in = response.expires_in();

        if (response.has_user()) {
            result.user_id = response.user().id();
            result.user_login = response.user().login();
            result.user_role = response.user().role();
        }
    }

    return result;
}