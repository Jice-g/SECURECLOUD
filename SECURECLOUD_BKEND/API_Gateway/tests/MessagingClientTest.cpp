#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "MessagingClient.h"

class FakeMessagingService final : public messaging::MessagingService::Service {
public:
    grpc::Status SendMessage(grpc::ServerContext*,
        const messaging::ChatMessageRequest* request,
        messaging::ChatMessageResponse* response) override {
        lastSender = request->sender();
        lastRecipient = request->recipient();
        lastContent = request->content();

        if (simulateFailure) {
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Service indisponible");
        }

        response->set_success(true);
        response->set_message_id("msg_12345");
        return grpc::Status::OK;
    }

    std::string lastSender;
    std::string lastRecipient;
    std::string lastContent;
    bool simulateFailure = false;
};

class MessagingClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        grpc::ServerBuilder builder;
        builder.RegisterService(&fakeService_);
        server_ = builder.BuildAndStart();

        auto channel = server_->InProcessChannel(grpc::ChannelArguments());
        client_ = std::make_unique<MessagingClient>(channel);
    }

    void TearDown() override {
        server_->Shutdown();
    }

    FakeMessagingService fakeService_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<MessagingClient> client_;
};

TEST_F(MessagingClientTest, EnvoiMessageValide) {
    bool ok = client_->sendMessage("alice", "bob", "Salut Bob !");

    EXPECT_TRUE(ok);
    EXPECT_EQ(fakeService_.lastSender, "alice");
    EXPECT_EQ(fakeService_.lastRecipient, "bob");
    EXPECT_EQ(fakeService_.lastContent, "Salut Bob !");
}

TEST_F(MessagingClientTest, RejetteChampsVidesSansAppelerGrpc) {
    EXPECT_FALSE(client_->sendMessage("", "bob", "Salut"));
    EXPECT_FALSE(client_->sendMessage("alice", "", "Salut"));
    EXPECT_FALSE(client_->sendMessage("alice", "bob", ""));
    EXPECT_TRUE(fakeService_.lastSender.empty());
}

TEST_F(MessagingClientTest, GerePanneServeurGrpc) {
    fakeService_.simulateFailure = true;
    bool ok = client_->sendMessage("alice", "bob", "Salut");

    EXPECT_FALSE(ok);
}