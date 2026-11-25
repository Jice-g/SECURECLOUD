#pragma once

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "auth.grpc.pb.h"

struct LoginResult {
	grpc::Status status;
	std::string access_token;
	std::string refresh_token;
	std::string token_type;
	int32_t expires_in = 0;
	std::string user_id;
	std::string user_login;
	std::string user_role;
	std::string created_at;
};

class AuthClient {
public:
	explicit AuthClient(std::shared_ptr<grpc::Channel> channel);
	bool validateToken(const std::string& token);
	LoginResult login(const std::string& loginStr, const std::string& password, const std::string& mfaCode = "");

private:
	std::unique_ptr<auth::AuthService::Stub> stub_;
};