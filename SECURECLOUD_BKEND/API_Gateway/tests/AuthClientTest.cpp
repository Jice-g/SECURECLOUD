#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "AuthClient.h"

class FakeService : public auth::AuthService::Service {
public:
    grpc::Status ValidateToken(grpc::ServerContext*, const auth::TokenRequest* request, auth::TokenResponse* response) override
    {
        response->set_is_valid(request->token() == "valid_token");
        return grpc::Status::OK;
    }
};

class AuthClientTest : public ::testing::Test {
protected:
    FakeService fake_service;
    std::unique_ptr<grpc::Server> server;
    std::unique_ptr<AuthClient> client;

    void SetUp() override {
        grpc::ServerBuilder builder;
        builder.RegisterService(&fake_service);
        server = builder.BuildAndStart(); 
        ASSERT_TRUE(server != nullptr);

        auto channel = server->InProcessChannel(grpc::ChannelArguments());
        client = std::make_unique<AuthClient>(channel);
    }

    void TearDown() override {
        server->Shutdown();
    }
};

TEST_F(AuthClientTest, TokenValide) {
    EXPECT_TRUE(client->validateToken("valid_token"));
}

TEST_F(AuthClientTest, TokenInvalide) {
    EXPECT_FALSE(client->validateToken("bad_token"));
}