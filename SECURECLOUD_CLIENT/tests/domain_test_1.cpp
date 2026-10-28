#include <functional>
#include <string>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

#include "domain/model/Enums.h"
#include "domain/model/UserId.h"
#include "domain/model/MessageId.h"
#include "domain/model/ConversationId.h"
#include "domain/model/AttachmentId.h"
#include "domain/model/UserLogin.h"
#include "domain/model/MessageContent.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/User.h"
#include "domain/model/Conversation.h"
#include "domain/model/Message.h"

#include "domain_test.h"

using namespace domain;

// =============================================================================
// Helpers
// =============================================================================

static Message makeMessage(const std::string& text = "Hello")
{
    return Message(MessageId("m1"), ConversationId("c1"), UserId("u1"),
                   MessageContent(text), TimeStamp::now(), Priority::STANDARD);
}

/// Builds a message in the requested status by following legal transitions only.
static Message makeMessageIn(MessageStatus status) {
    Message m = makeMessage("Test de MessageStatus");
    switch (status) {
    case MessageStatus::DRAFTED:     break;
    case MessageStatus::BUFFERED:    m.markAsBuffered(); break;
    case MessageStatus::SENDING:     m.markAsBuffered(); m.markAsSending(); break;
    case MessageStatus::SENT:        m.markAsBuffered(); m.markAsSending(); m.markAsSent(); break;
    case MessageStatus::DELIVERED:   m.markAsBuffered(); m.markAsSending(); m.markAsSent(); m.markAsDelivered(); break;
    case MessageStatus::READ:        m.markAsBuffered(); m.markAsSending(); m.markAsSent(); m.markAsDelivered(); m.markAsRead(); break;
    case MessageStatus::UNDELIVERED: m.markAsBuffered(); m.markAsSending(); m.markAsUndelivered(); m.markAsBuffered(); break;
    }
    return m;
}

// =============================================================================
// IDs (UserId, MessageId, ConversationId, AttachmentId)
// =============================================================================

void test_UserId_ValidConstruction()
{ CHECK(UserId("user-001").value() == "user-001"); }

void test_UserId_RejectsEmpty()      { CHECK_THROWS(UserId(""), std::invalid_argument); }
void test_UserId_Equality() {
    UserId a("alice"), b("alice"), c("bob");
    CHECK(a == b); CHECK(!(a == c)); CHECK(a != c); CHECK(!(a != b));
}
void test_UserId_IsCaseSensitive() { CHECK(UserId("Alice") != UserId("alice")); }
void test_UserId_HashUsableInUnorderedSet() {
    std::unordered_set<UserId> set;
    set.insert(UserId("a")); set.insert(UserId("a")); set.insert(UserId("b"));
    CHECK(set.size() == 2);
    CHECK(set.count(UserId("a")) == 1);
    CHECK(std::hash<UserId>{}(UserId("a")) == std::hash<UserId>{}(UserId("a")));
}

void test_MessageId_Rules() {
    CHECK(MessageId("m1").value() == "m1");
    CHECK_THROWS(MessageId(""), std::invalid_argument);
    CHECK(MessageId("x") == MessageId("x"));
    CHECK(MessageId("x") != MessageId("y"));
    std::unordered_set<MessageId> s; s.insert(MessageId("x")); s.insert(MessageId("x"));
    CHECK(s.size() == 1);
}
void test_ConversationId_Rules() {
    CHECK(ConversationId("c1").value() == "c1");
    CHECK_THROWS(ConversationId(""), std::invalid_argument);
    CHECK(ConversationId("x") == ConversationId("x"));
    CHECK(ConversationId("x") != ConversationId("y"));
    std::unordered_set<ConversationId> s; s.insert(ConversationId("x")); s.insert(ConversationId("x"));
    CHECK(s.size() == 1);
}
void test_AttachmentId_Rules() {
    CHECK(AttachmentId("a1").value() == "a1");
    CHECK_THROWS(AttachmentId(""), std::invalid_argument);
    CHECK(AttachmentId("x") == AttachmentId("x"));
    CHECK(AttachmentId("x") != AttachmentId("y"));
    std::unordered_set<AttachmentId> s; s.insert(AttachmentId("x")); s.insert(AttachmentId("x"));
    CHECK(s.size() == 1);
}

// =============================================================================
// UserLogin
// =============================================================================

void test_UserLogin_Normalization() {
    CHECK(UserLogin("  Alice  ").value() == "alice");
    CHECK(UserLogin("ALICE").value() == "alice");
}
void test_UserLogin_InternalWhitespaceIsRemoved() {
    // Documents the CURRENT behavior ("al ice" -> "alice"). See review: to be decided.
    CHECK(UserLogin("al ice").value() == "alice");
}
void test_UserLogin_RejectsInvalidChar() {
    CHECK_THROWS(UserLogin("alice@msf"), std::invalid_argument);
    CHECK_THROWS(UserLogin("al!ce"),     std::invalid_argument);
    CHECK_THROWS(UserLogin("jos\xC3\xA9"), std::invalid_argument);   // 'é' (UTF-8)
}
void test_UserLogin_AllowedSpecialChars() {
    CHECK_NOTHROW(UserLogin("a_b-c.d"));
    CHECK(UserLogin("jcg_1").value() == "jcg_1");
}
void test_UserLogin_LengthBoundaries() {
    CHECK_THROWS(UserLogin("ab"), std::invalid_argument);
    CHECK_NOTHROW(UserLogin("abc"));
    CHECK_NOTHROW(UserLogin(std::string(32, 'a')));
    CHECK_THROWS(UserLogin(std::string(33, 'a')), std::invalid_argument);
    CHECK_THROWS(UserLogin(""), std::invalid_argument);
    CHECK_THROWS(UserLogin("   "), std::invalid_argument);
}
void test_UserLogin_EqualityCaseInsensitive() {
    CHECK(UserLogin("Alice") == UserLogin("ALICE"));
    CHECK(UserLogin("alice") != UserLogin("bob"));
}
void test_UserLogin_AssignmentValid() {
    UserLogin l("alice");
    l = "  BOB  ";
    CHECK(l.value() == "bob");
}
void test_UserLogin_AssignmentInvalidKeepsOldValue() {
    UserLogin l("alice");
    CHECK_THROWS(l = std::string("a!"), std::invalid_argument);
    CHECK(l.value() == "alice");
    CHECK_THROWS(l = std::string("ab"), std::invalid_argument);
    CHECK(l.value() == "alice");
}

// =============================================================================
// MessageContent
// =============================================================================

void test_MessageContent_TrailingTrimmed()  { CHECK(MessageContent("Hello   \n\t").value() == "Hello"); }
void test_MessageContent_LeadingPreserved() { CHECK(MessageContent("   Hello").value() == "   Hello"); }
void test_MessageContent_InternalPreserved(){ CHECK(MessageContent("a  b\nc").value() == "a  b\nc"); }
void test_MessageContent_EmptyAllowed()     { CHECK(MessageContent("").isEmpty()); CHECK(MessageContent("").length() == 0); }
void test_MessageContent_WhitespaceOnlyIsEmpty() { CHECK(MessageContent("   \t\n   ").isEmpty()); }
void test_MessageContent_NotEmpty()         { CHECK(!MessageContent("x").isEmpty()); }
void test_MessageContent_LengthBoundary() {
    CHECK_NOTHROW(MessageContent(std::string(4096, 'a')));
    CHECK(MessageContent(std::string(4096, 'a')).length() == 4096);
    CHECK_THROWS(MessageContent(std::string(4097, 'a')), std::invalid_argument);
}
void test_MessageContent_TrimHappensBeforeLengthCheck() {
    // 4096 chars + trailing spaces: accepted, because the limit applies to the trimmed text.
    CHECK_NOTHROW(MessageContent(std::string(4096, 'a') + "     "));
}
void test_MessageContent_RejectsNulChar() {
    CHECK_THROWS(MessageContent(std::string("A\0B", 3)), std::invalid_argument);
}
void test_MessageContent_LengthCountsBytesNotCharacters() {
    // Documents the CURRENT behavior: "é" = 2 bytes in UTF-8. See review.
    CHECK(MessageContent("\xC3\xA9").length() == 2);
}
void test_MessageContent_Equality() {
    CHECK(MessageContent("a") == MessageContent("a  "));
    CHECK(MessageContent("a") != MessageContent("b"));
}

// =============================================================================
// TimeStamp
// =============================================================================

void test_TimeStamp_NowAndCopy() {
    TimeStamp t = TimeStamp::now(); TimeStamp t2(t);
    CHECK(t == t2); CHECK(!(t != t2));
}
void test_TimeStamp_Ordering() {
    auto base = std::chrono::system_clock::now();
    TimeStamp early(base), late(base + std::chrono::seconds(1));
    CHECK(early < late); CHECK(late > early); CHECK(early <= early); CHECK(early != late);
    CHECK(late.value() - early.value() == std::chrono::seconds(1));
}

// =============================================================================
// User
// =============================================================================

static User makeUser(UserRole r = UserRole::STD_USER) {
    return User(UserId("u1"), UserLogin("alice"), r, TimeStamp::now());
}

void test_User_Construction() {
    TimeStamp created = TimeStamp::now();
    User u(UserId("u1"), UserLogin("alice"), UserRole::STD_USER, created);
    CHECK(u.Id() == UserId("u1"));
    CHECK(u.Login() == UserLogin("alice"));
    CHECK(u.Role() == UserRole::STD_USER);
    CHECK(u.Presence() == OnlineStatus::OFFLINE);
    CHECK(u.CreatedAt() == created);
    CHECK(!u.IsOnline());
}
void test_User_InitialPresenceCanBeSet() {
    User u(UserId("u1"), UserLogin("alice"), UserRole::ADMIN, TimeStamp::now(), OnlineStatus::ONLINE);
    CHECK(u.IsOnline());
}
void test_User_IsOfflineIsConsistent() {
    User u = makeUser();
    CHECK(u.IsOffline()); CHECK(!u.IsOnline());
    u.Online();
    CHECK(!u.IsOffline()); CHECK(u.IsOnline());
    u.Away();
    CHECK(!u.IsOffline()); CHECK(!u.IsOnline());
}
void test_User_LastLoginEmptyUntilOnline() {
    User u = makeUser();
    CHECK(!u.LastLogin().has_value());
    u.Offline();                                  // going offline is not a login
    CHECK(!u.LastLogin().has_value());
}
void test_User_OnlineSetsLastLogin() {
    User u = makeUser();
    u.Online();
    CHECK(u.IsOnline());
    CHECK(u.LastLogin().has_value());
}
void test_User_OfflineDoesNotChangeLastLogin() {
    User u = makeUser();
    u.Online();
    auto before = *u.LastLogin();
    u.Offline();
    CHECK(u.IsOffline());
    CHECK(*u.LastLogin() == before);
}
void test_User_AwayOnlyFromOnline() {
    User u = makeUser();
    u.Away();                                     // OFFLINE -> ignored
    CHECK(u.Presence() == OnlineStatus::OFFLINE);
    u.Online(); u.Away();
    CHECK(u.Presence() == OnlineStatus::AWAY);
}
void test_User_AwayToOnlineIsNotANewLogin() {
    User u = makeUser();
    u.Online();
    auto first = *u.LastLogin();
    u.Away(); u.Online();
    CHECK(u.IsOnline());
    CHECK(*u.LastLogin() == first);
}
void test_User_RoleChange() {
    User u = makeUser(UserRole::STD_USER);
    u.setRole(UserRole::ORGA);
    CHECK(u.Role() == UserRole::ORGA);
    CHECK(makeUser(UserRole::ADMIN).Role() == UserRole::ADMIN);
}

// =============================================================================
// Conversations
// =============================================================================

void test_DirectConversation_Construction() {
    DirectConversation c(ConversationId("c1"), UserId("u1"), UserId("u2"), TimeStamp::now());
    CHECK(c.type() == ConversationType::DIRECT);
    CHECK(c.id() == ConversationId("c1"));
    CHECK(c.participant1() == UserId("u1"));
    CHECK(c.participant2() == UserId("u2"));
    CHECK(c.IsParticipating(UserId("u1")));
    CHECK(c.IsParticipating(UserId("u2")));
    CHECK(!c.IsParticipating(UserId("u3")));
}
void test_DirectConversation_RejectsSelfChat() {
    CHECK_THROWS(DirectConversation(ConversationId("c1"), UserId("u1"), UserId("u1"), TimeStamp::now()),
                 std::invalid_argument);
}
void test_DirectConversation_SamePairIsOrderInsensitive() {
    DirectConversation ab(ConversationId("1"), UserId("a"), UserId("b"), TimeStamp::now());
    DirectConversation ba(ConversationId("2"), UserId("b"), UserId("a"), TimeStamp::now());
    DirectConversation ac(ConversationId("3"), UserId("a"), UserId("c"), TimeStamp::now());
    CHECK(ab.samePair(ba)); CHECK(ba.samePair(ab)); CHECK(!ab.samePair(ac));
}
void test_Conversation_PolymorphicUse() {
    std::vector<std::unique_ptr<Conversation>> v;
    v.push_back(std::make_unique<DirectConversation>(ConversationId("d"), UserId("u1"), UserId("u2"), TimeStamp::now()));
    auto g = std::make_unique<GroupConversation>(ConversationId("g"), "Team", TimeStamp::now());
    g->addParticipant(UserId("u1"));
    v.push_back(std::move(g));
    CHECK(v[0]->type() == ConversationType::DIRECT);
    CHECK(v[1]->type() == ConversationType::GROUP);
    CHECK(v[0]->IsParticipating(UserId("u2")));
    CHECK(!v[1]->IsParticipating(UserId("u2")));
}
void test_GroupConversation_Construction() {
    GroupConversation c(ConversationId("c2"), "Project MSF", TimeStamp::now());
    CHECK(c.type() == ConversationType::GROUP);
    CHECK(c.title() == "Project MSF");
    CHECK(c.participants().empty());
}
void test_GroupConversation_RejectsEmptyTitle() {
    CHECK_THROWS(GroupConversation(ConversationId("c2"), "", TimeStamp::now()), std::invalid_argument);
}
void test_GroupConversation_AddParticipant() {
    GroupConversation c(ConversationId("c2"), "T", TimeStamp::now());
    c.addParticipant(UserId("u1")); c.addParticipant(UserId("u2"));
    CHECK(c.IsParticipating(UserId("u1"))); CHECK(c.IsParticipating(UserId("u2")));
    CHECK(c.participants().size() == 2);
}
void test_GroupConversation_RejectsDuplicateParticipant() {
    GroupConversation c(ConversationId("c2"), "T", TimeStamp::now());
    c.addParticipant(UserId("u1"));
    CHECK_THROWS(c.addParticipant(UserId("u1")), std::invalid_argument);
    CHECK(c.participants().size() == 1);
}
void test_GroupConversation_RemoveParticipant() {
    GroupConversation c(ConversationId("c2"), "T", TimeStamp::now());
    c.addParticipant(UserId("u1")); c.addParticipant(UserId("u2"));
    c.removeParticipant(UserId("u1"));
    CHECK(!c.IsParticipating(UserId("u1"))); CHECK(c.IsParticipating(UserId("u2")));
}
void test_GroupConversation_RejectsRemovingUnknownParticipant() {
    GroupConversation c(ConversationId("c2"), "T", TimeStamp::now());
    CHECK_THROWS(c.removeParticipant(UserId("ghost")), std::invalid_argument);
}

// =============================================================================
// Message
// =============================================================================

void test_Message_StartsBuffered() {
    Message m = makeMessage();
    CHECK(m.status() == MessageStatus::BUFFERED);
    CHECK(!m.sentAt().has_value());
    CHECK(m.id() == MessageId("m1"));
    CHECK(m.conversationId() == ConversationId("c1"));
    CHECK(m.senderId() == UserId("u1"));
    CHECK(m.content().value() == "Hello");
    CHECK(m.priority() == Priority::STANDARD);
    CHECK(!m.hasAttachments());
}
void test_Message_FullLifecycle() {
    Message m = makeMessage();
    m.markAsSending();   CHECK(m.status() == MessageStatus::SENDING);
    m.markAsSent();      CHECK(m.status() == MessageStatus::SENT);
    m.markAsDelivered(); CHECK(m.status() == MessageStatus::DELIVERED);
    m.markAsRead();      CHECK(m.status() == MessageStatus::READ);
}
void test_Message_MarkAsSentSetsSentAt() {
    Message m = makeMessage();
    m.markAsSending();
    CHECK(!m.sentAt().has_value());
    m.markAsSent();
    CHECK(m.sentAt().has_value());
}
void test_Message_FailedMarkAsSentDoesNotSetSentAt() {
    Message m = makeMessage();                       // BUFFERED: markAsSent is illegal
    CHECK_THROWS(m.markAsSent(), std::logic_error);
    CHECK(!m.sentAt().has_value());
    CHECK(m.status() == MessageStatus::BUFFERED);
}
void test_Message_UndeliveredFromSendingOrSent() {
    CHECK_NOTHROW(makeMessageIn(MessageStatus::SENDING).markAsUndelivered());
    CHECK_NOTHROW(makeMessageIn(MessageStatus::SENT).markAsUndelivered());
    Message m = makeMessageIn(MessageStatus::SENDING); m.markAsUndelivered();
    CHECK(m.status() == MessageStatus::UNDELIVERED);
}

// Exhaustive transition matrix: 6 states x 5 operations.
void test_Message_TransitionMatrix() {
    struct Op { const char* name; std::function<void(Message&)> run; std::vector<MessageStatus> legalFrom; };
    const std::vector<Op> ops = {
        {"markAsSending",     [](Message& m){ m.markAsSending(); },     {MessageStatus::BUFFERED}},
        {"markAsSent",        [](Message& m){ m.markAsSent(); },        {MessageStatus::SENDING}},
        {"markAsDelivered",   [](Message& m){ m.markAsDelivered(); },   {MessageStatus::SENT}},
        {"markAsUndelivered", [](Message& m){ m.markAsUndelivered(); }, {MessageStatus::SENDING, MessageStatus::SENT}},
        {"markAsRead",        [](Message& m){ m.markAsRead(); },        {MessageStatus::DELIVERED}},
    };
    const std::vector<MessageStatus> states = {
        MessageStatus::BUFFERED, MessageStatus::SENDING, MessageStatus::SENT,
        MessageStatus::DELIVERED, MessageStatus::UNDELIVERED, MessageStatus::READ};

    for (const auto& op : ops) {
        for (auto st : states) {
            Message m = makeMessageIn(st);
            bool legal = std::find(op.legalFrom.begin(), op.legalFrom.end(), st) != op.legalFrom.end();
            bool threw = false;
            try { op.run(m); } catch (const std::logic_error&) { threw = true; }
            if (threw == legal) {
                std::cerr << "\n    transition " << op.name << " from state #" << static_cast<int>(st)
                          << (legal ? " should be legal" : " should be refused");
            }
            CHECK(threw != legal);
            if (threw) CHECK(m.status() == st);       // a refused transition leaves the state intact
        }
    }
}

void test_Message_EditContentOnlyIfBuffered() {
    Message m = makeMessage();
    m.editContent(MessageContent("Updated"));
    CHECK(m.content().value() == "Updated");
    m.markAsSending();
    CHECK_THROWS(m.editContent(MessageContent("Nope")), std::logic_error);
    CHECK(m.content().value() == "Updated");
}
void test_Message_AddAndRemoveAttachment() {
    Message m = makeMessage("");
    m.addAttachment(AttachmentId("a1")); m.addAttachment(AttachmentId("a2"));
    CHECK(m.hasAttachments()); CHECK(m.attachments().size() == 2);
    m.removeAttachment(AttachmentId("a1"));
    CHECK(m.attachments().size() == 1);
    CHECK(m.attachments()[0] == AttachmentId("a2"));
}
void test_Message_RejectsDuplicateAttachment() {
    Message m = makeMessage();
    m.addAttachment(AttachmentId("a1"));
    CHECK_THROWS(m.addAttachment(AttachmentId("a1")), std::invalid_argument);
    CHECK(m.attachments().size() == 1);
}
void test_Message_RejectsRemovingUnknownAttachment() {
    Message m = makeMessage();
    CHECK_THROWS(m.removeAttachment(AttachmentId("ghost")), std::invalid_argument);
}
void test_Message_AttachmentsFrozenAfterBuffered() {
    Message m = makeMessage(); m.addAttachment(AttachmentId("a1")); m.markAsSending();
    CHECK_THROWS(m.addAttachment(AttachmentId("a2")), std::logic_error);
    CHECK_THROWS(m.removeAttachment(AttachmentId("a1")), std::logic_error);
    CHECK(m.attachments().size() == 1);
}
void test_Message_ContentOrAttachmentInvariant() {
    Message empty = makeMessage("");
    CHECK(!empty.isValid());
    Message blank = makeMessage("   \n ");
    CHECK(!blank.isValid());
    Message text = makeMessage("Hi");
    CHECK(text.isValid());
    empty.addAttachment(AttachmentId("a1"));
    CHECK(empty.isValid());
    empty.removeAttachment(AttachmentId("a1"));
    CHECK(!empty.isValid());
}
void test_Message_CopyIsIndependent() {
    Message a = makeMessage();
    Message b = a;
    b.markAsSending();
    CHECK(a.status() == MessageStatus::BUFFERED);
    CHECK(b.status() == MessageStatus::SENDING);
}

// =============================================================================
// MAIN
// =============================================================================

int main(int argc, char** argv) {
    std::cout << "=====================================================\n"
              << "SecureCloud Client - Domain Test Suite\n"
              << "=====================================================\n";

    RUN_TEST(test_UserId_ValidConstruction); RUN_TEST(test_UserId_RejectsEmpty);
    RUN_TEST(test_UserId_Equality); RUN_TEST(test_UserId_IsCaseSensitive);
    RUN_TEST(test_UserId_HashUsableInUnorderedSet);
    RUN_TEST(test_MessageId_Rules); RUN_TEST(test_ConversationId_Rules); RUN_TEST(test_AttachmentId_Rules);

    RUN_TEST(test_UserLogin_Normalization); RUN_TEST(test_UserLogin_InternalWhitespaceIsRemoved);
    RUN_TEST(test_UserLogin_RejectsInvalidChar); RUN_TEST(test_UserLogin_AllowedSpecialChars);
    RUN_TEST(test_UserLogin_LengthBoundaries); RUN_TEST(test_UserLogin_EqualityCaseInsensitive);
    RUN_TEST(test_UserLogin_AssignmentValid); RUN_TEST(test_UserLogin_AssignmentInvalidKeepsOldValue);

    RUN_TEST(test_MessageContent_TrailingTrimmed); RUN_TEST(test_MessageContent_LeadingPreserved);
    RUN_TEST(test_MessageContent_InternalPreserved); RUN_TEST(test_MessageContent_EmptyAllowed);
    RUN_TEST(test_MessageContent_WhitespaceOnlyIsEmpty); RUN_TEST(test_MessageContent_NotEmpty);
    RUN_TEST(test_MessageContent_LengthBoundary); RUN_TEST(test_MessageContent_TrimHappensBeforeLengthCheck);
    RUN_TEST(test_MessageContent_RejectsNulChar); RUN_TEST(test_MessageContent_LengthCountsBytesNotCharacters);
    RUN_TEST(test_MessageContent_Equality);

    RUN_TEST(test_TimeStamp_NowAndCopy); RUN_TEST(test_TimeStamp_Ordering);

    RUN_TEST(test_User_Construction); RUN_TEST(test_User_InitialPresenceCanBeSet);
    RUN_TEST(test_User_IsOfflineIsConsistent); RUN_TEST(test_User_LastLoginEmptyUntilOnline);
    RUN_TEST(test_User_OnlineSetsLastLogin); RUN_TEST(test_User_OfflineDoesNotChangeLastLogin);
    RUN_TEST(test_User_AwayOnlyFromOnline); RUN_TEST(test_User_AwayToOnlineIsNotANewLogin);
    RUN_TEST(test_User_RoleChange);

    RUN_TEST(test_DirectConversation_Construction); RUN_TEST(test_DirectConversation_RejectsSelfChat);
    RUN_TEST(test_DirectConversation_SamePairIsOrderInsensitive); RUN_TEST(test_Conversation_PolymorphicUse);
    RUN_TEST(test_GroupConversation_Construction); RUN_TEST(test_GroupConversation_RejectsEmptyTitle);
    RUN_TEST(test_GroupConversation_AddParticipant); RUN_TEST(test_GroupConversation_RejectsDuplicateParticipant);
    RUN_TEST(test_GroupConversation_RemoveParticipant); RUN_TEST(test_GroupConversation_RejectsRemovingUnknownParticipant);

    RUN_TEST(test_Message_StartsBuffered); RUN_TEST(test_Message_FullLifecycle);
    RUN_TEST(test_Message_MarkAsSentSetsSentAt); RUN_TEST(test_Message_FailedMarkAsSentDoesNotSetSentAt);
    RUN_TEST(test_Message_UndeliveredFromSendingOrSent); RUN_TEST(test_Message_TransitionMatrix);
    RUN_TEST(test_Message_EditContentOnlyIfBuffered); RUN_TEST(test_Message_AddAndRemoveAttachment);
    RUN_TEST(test_Message_RejectsDuplicateAttachment); RUN_TEST(test_Message_RejectsRemovingUnknownAttachment);
    RUN_TEST(test_Message_AttachmentsFrozenAfterBuffered); RUN_TEST(test_Message_ContentOrAttachmentInvariant);
    RUN_TEST(test_Message_CopyIsIndependent);

    std::cout << "\n=====================================================\n"
              << g_passedTests.size() << " passed, " << g_failedTests.size() << " failed, "
              << g_totalChecks << " checks.\n"
              << "=====================================================\n";

    const std::string reportPath = (argc > 1) ? argv[1] : "domain_report.txt";
    writeReport(reportPath, "Domain Tests Report");

    return g_failedTests.empty() ? 0 : 1;   // non-zero exit code => CI fails
}
