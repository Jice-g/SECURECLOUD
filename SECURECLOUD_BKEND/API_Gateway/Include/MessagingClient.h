#pragma once
#include <grpcpp/grpcpp.h>
#include <chrono>
#include <memory>
#include <string>
#include "messaging.grpc.pb.h"

class MessagingClient {
public:
	//Explicit for constructor to avoid conversion
    explicit MessagingClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(messaging::MessagingService::NewStub(channel)) {
    }

    bool sendMessage(const std::string& sender,
        const std::string& recipient,
        const std::string& content) {
        if (sender.empty() || recipient.empty() || content.empty()) {
            return false;
        }

        messaging::ChatMessageRequest request;
        request.set_sender(sender);
        request.set_recipient(recipient);
        request.set_content(content);

        messaging::ChatMessageResponse response;
        grpc::ClientContext context;

        // Timeout de 2s
        auto time = std::chrono::system_clock::now() + std::chrono::milliseconds(2000);
        context.set_deadline(time);

        grpc::Status status = stub_->SendMessage(&context, request, &response);

        if (!status.ok()) {
            return false;
        }

        return response.success();
    }

private:
    std::unique_ptr<messaging::MessagingService::Stub> stub_;
};