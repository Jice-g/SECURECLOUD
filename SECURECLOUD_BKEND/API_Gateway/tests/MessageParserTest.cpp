#include <gtest/gtest.h>
#include "MessageParser.h"

TEST(MessageParserTest, ParseAction) {
    MessageParser parser;
    std::string json = R"({"action":"send_message","content":"Bonjour"})";

    std::string action = parser.parseAction(json);

    EXPECT_EQ(action, "send_message");
    EXPECT_EQ(parser.getLastAction(), "send_message");
}

TEST(MessageParserTest, JsonInvalide) {

    MessageParser parser;
    std::string action = parser.parseAction("{json_mal_forme");
    EXPECT_TRUE(action.empty());
    EXPECT_TRUE(parser.getLastAction().empty());
}

TEST(MessageParserTest, ActionChampQuiManque) {
    MessageParser parser;

    EXPECT_TRUE(parser.parseAction(R"({"sender":"user1"})").empty());
    EXPECT_TRUE(parser.parseAction(R"({"action":12345})").empty());
}

TEST(MessageParserTest, ParseMessageValide) {
    MessageParser parser;
    std::string json = R"({
        "action": "send_message",
        "sender": "alice",
        "recipient": "bob",
        "content": "Salut Bob !"
    })";

    ParsedMessage msg = parser.parseMessage(json);

    EXPECT_TRUE(msg.isValid);
    EXPECT_EQ(msg.action, "send_message");
    EXPECT_EQ(msg.sender, "alice");
    EXPECT_EQ(msg.recipient, "bob");
    EXPECT_EQ(msg.content, "Salut Bob !");
    EXPECT_EQ(parser.getLastAction(), "send_message");
}

TEST(MessageParserTest, ParseMessageChampManquant) {
    MessageParser parser;
    // Il manque recipient
    std::string json = R"({
        "action": "send_message",
        "sender": "alice",
        "content": "Salut ?"
    })";

    ParsedMessage msg = parser.parseMessage(json);

    EXPECT_FALSE(msg.isValid);
}