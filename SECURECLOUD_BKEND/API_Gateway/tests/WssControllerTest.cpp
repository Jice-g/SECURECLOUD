#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include <crow/json.h>
#include "WssController.h"

class FakeWssMessagingService final : public messaging::MessagingService::Service {
public:
    grpc::Status SendMessage(grpc::ServerContext*,
        const messaging::ChatMessageRequest* request,
        messaging::ChatMessageResponse* response) override {
        if (simulateFailure) {
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Service down");
        }
        receivedSender = request->sender();
        receivedRecipient = request->recipient();
        receivedContent = request->content();

        response->set_success(true);
        response->set_message_id("msg_999");
        return grpc::Status::OK;
    }

    std::string receivedSender;
    std::string receivedRecipient;
    std::string receivedContent;
    bool simulateFailure = false;
};

class WssControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        grpc::ServerBuilder builder;
        builder.RegisterService(&fakeService_);
        server_ = builder.BuildAndStart();

        auto channel = server_->InProcessChannel(grpc::ChannelArguments());
        auto client = std::make_shared<MessagingClient>(channel);
        controller_ = std::make_unique<WssController>(client);
    }

    void TearDown() override {
        server_->Shutdown();
    }

    FakeWssMessagingService fakeService_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<WssController> controller_;
};

TEST_F(WssControllerTest, CompletValide) {
    std::string jsonInput = R"({
        "action": "send_message",
        "sender": "alice",
        "recipient": "bob",
        "content": "Bonjour depuis le WebSocket !"
    })";

    std::string rawReply = controller_->handleIncomingText(jsonInput);
    auto reply = crow::json::load(rawReply);

    ASSERT_TRUE(reply);
    EXPECT_EQ(std::string(reply["status"].s()), "ok");
    EXPECT_EQ(fakeService_.receivedSender, "alice");
    EXPECT_EQ(fakeService_.receivedRecipient, "bob");
    EXPECT_EQ(fakeService_.receivedContent, "Bonjour depuis le WebSocket !");
}

TEST_F(WssControllerTest, JsonIncompletS) {
    std::string badJson = R"({"action": "send_message", "sender": "alice"})";

    std::string rawReply = controller_->handleIncomingText(badJson);
    auto reply = crow::json::load(rawReply);

    ASSERT_TRUE(reply);
    EXPECT_EQ(std::string(reply["status"].s()), "error");
    EXPECT_TRUE(fakeService_.receivedSender.empty());
}

TEST_F(WssControllerTest, ActionInconnue) {
    std::string unknownAction = R"({
        "action": "delete_all",
        "sender": "alice",
        "recipient": "bob",
        "content": "test"
    })";

    std::string rawReply = controller_->handleIncomingText(unknownAction);
    auto reply = crow::json::load(rawReply);

    ASSERT_TRUE(reply);
    EXPECT_EQ(std::string(reply["status"].s()), "error");
    EXPECT_EQ(std::string(reply["message"].s()), "Action inconnue");
}

TEST_F(WssControllerTest, ErreurMicroservice) {
    fakeService_.simulateFailure = true;

    std::string jsonInput = R"({
        "action": "send_message",
        "sender": "alice",
        "recipient": "bob",
        "content": "Test panne"
    })";

    std::string rawReply = controller_->handleIncomingText(jsonInput);
    auto reply = crow::json::load(rawReply);

    ASSERT_TRUE(reply);
    EXPECT_EQ(std::string(reply["status"].s()), "error");
}