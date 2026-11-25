#pragma once
#include <crow/json.h>
#include <string>

struct ParsedMessage {
    std::string action;
    std::string sender;
    std::string recipient;
    std::string content;
    bool isValid = false;
};

class MessageParser {
public:
    MessageParser() = default;

	//Save the last action
    std::string parseAction(const std::string& rawJson) {
        lastAction.clear();

        //If texte corrupte finish
        auto json = crow::json::load(rawJson);
        if (!json) {
            return "";
        }

        //If action is string save the action
        if (json.has("action") && json["action"].t() == crow::json::type::String) {
            lastAction = std::string(json["action"].s());
        }

        return lastAction;
    }

    ParsedMessage parseMessage(const std::string& rawJson) {
        ParsedMessage result;

        auto json = crow::json::load(rawJson);
        if (!json) {
            lastAction.clear();
            return result;
        }

        //If one variable is not string or missing cler lastAction return result
        if (json.has("action") && json["action"].t() == crow::json::type::String) {
            result.action = std::string(json["action"].s());
            lastAction = result.action;
        }
        else {
            lastAction.clear();
        }

        if (json.has("sender") && json["sender"].t() == crow::json::type::String) {
            result.sender = std::string(json["sender"].s());
        }
        if (json.has("recipient") && json["recipient"].t() == crow::json::type::String) {
            result.recipient = std::string(json["recipient"].s());
        }
        if (json.has("content") && json["content"].t() == crow::json::type::String) {
            result.content = std::string(json["content"].s());
        }

        result.isValid = !result.action.empty() &&
            !result.sender.empty() &&
            !result.recipient.empty() &&
            !result.content.empty();

        return result;
    }

    std::string getLastAction() const { return lastAction; }

private:
    std::string lastAction;
};