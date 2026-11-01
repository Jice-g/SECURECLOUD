#include <functional>
#include <string>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

#include "domain_test.h"

using namespace domain;



// =============================================================================
// Helpers
// =============================================================================


/// @brief Makes a new Message in order to test it
Message makeMessage(const std::string& text = "Hello")
{
    return Message(MessageId("m1"), ConversationId("c1"), UserId("u1"),
                   MessageContent(text), TimeStamp::now(), Priority::NORMAL);
}


/// @brief Builds a message in the requested status by following legal transitions.
static Message makeMessageIn(MessageStatus status) {
    Message m = makeMessage();
    switch (status) {
    case MessageStatus::DRAFTED:     break;
    case MessageStatus::BUFFERED:    m.markAsBuffered(); break;
    case MessageStatus::SENDING:     m.markAsBuffered(); m.markAsSending(); break;
    case MessageStatus::SENT:        m.markAsBuffered(); m.markAsSending(); m.markAsSent(); break;
    case MessageStatus::DELIVERED:   m.markAsBuffered(); m.markAsSending(); m.markAsSent(); m.markAsDelivered(); break;
    case MessageStatus::READ:        m.markAsBuffered(); m.markAsSending(); m.markAsSent(); m.markAsDelivered(); m.markAsRead(); break;
    case MessageStatus::UNDELIVERED: m.markAsBuffered(); m.markAsSending(); m.markAsUndelivered(); break;
    }
    return m;
}

// =============================================================================
// IDs (UserId, MessageId, ConversationId, AttachmentId)
// =============================================================================

void test_UserId_ValidConstruction()
{ CHECK(UserId("user-001").value() == "user-001"); }

void test_UserId_RejectsEmpty()
{ CHECK_THROWS(UserId(""), std::invalid_argument); }


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

void test_UserLogin_InternalWhitespaceIsRejected() {
    // Decision 30/10/2026: only leading/trailing whitespace is trimmed; an internal one is an error.
    CHECK_THROWS(UserLogin("al ice"),  std::invalid_argument);
    CHECK_THROWS(UserLogin("al\tice"), std::invalid_argument);
    CHECK_THROWS(UserLogin(" al ice "), std::invalid_argument);
}
void test_UserLogin_TrimsWhitespace() {
    CHECK(UserLogin("\talice\n").value() == "alice");
    CHECK(UserLogin(" \r\n alice \t ").value() == "alice");
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
    CHECK(u.id() == UserId("u1"));
    CHECK(u.login() == UserLogin("alice"));
    CHECK(u.role() == UserRole::STD_USER);
    CHECK(u.presence() == OnlineStatus::OFFLINE);
    CHECK(u.createdAt() == created);
    CHECK(!u.isOnline());
}
void test_User_InitialPresenceCanBeSet() {
    User u(UserId("u1"), UserLogin("alice"), UserRole::ADMIN, TimeStamp::now(), OnlineStatus::ONLINE);
    CHECK(u.isOnline());
}
void test_User_IsOfflineIsConsistent() {
    User u = makeUser();
    CHECK(u.isOffline()); CHECK(!u.isOnline());
    u.onLine();
    CHECK(!u.isOffline()); CHECK(u.isOnline());
    u.away();
    CHECK(!u.isOffline()); CHECK(!u.isOnline());
}

void test_User_AwayOnlyFromOnline() {
    User u = makeUser();
    u.away();                                     // OFFLINE -> ignored
    CHECK(u.presence() == OnlineStatus::OFFLINE);
    u.onLine(); u.away();
    CHECK(u.presence() == OnlineStatus::AWAY);
}

void test_User_RoleChange() {
    User u = makeUser(UserRole::STD_USER);
    u.setRole(UserRole::COORD);
    CHECK(u.role() == UserRole::COORD);
    CHECK(makeUser(UserRole::ADMIN).role() == UserRole::ADMIN);
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
    CHECK(c.isParticipating(UserId("u1")));
    CHECK(c.isParticipating(UserId("u2")));
    CHECK(!c.isParticipating(UserId("u3")));
}
void test_DirectConversation_SelfChatIsAllowed() {
    // Decision 30/10/2026: a user may write to himself ("notes to self").
    DirectConversation self(ConversationId("c-self"), UserId("u1"), UserId("u1"), TimeStamp::now());
    CHECK(self.type() == ConversationType::DIRECT);
    CHECK(self.isParticipating(UserId("u1")));
    CHECK(!self.isParticipating(UserId("u2")));
}
void test_DirectConversation_SamePairIsOrderInsensitive() {
    UserId a("a") ; UserId b("b"); UserId c("c") ;
    DirectConversation ab(ConversationId("1"), a, b, TimeStamp::now());
    DirectConversation ac(ConversationId("3"), a, c, TimeStamp::now());
    CHECK(ab.checkParticipants(a,b)) ; CHECK(ab.checkParticipants(b,a)) ;
    CHECK(!ac.checkParticipants(a,b)); CHECK(!ac.checkParticipants(b,a));
}
void test_Conversation_PolymorphicUse() {
    std::vector<std::unique_ptr<Conversation>> v;
    v.push_back(std::make_unique<DirectConversation>(ConversationId("d"), UserId("u1"), UserId("u2"), TimeStamp::now()));
    auto g = std::make_unique<GroupConversation>(ConversationId("g"), "Team", TimeStamp::now());
    g->addParticipant(UserId("u1"));
    v.push_back(std::move(g));
    CHECK(v[0]->type() == ConversationType::DIRECT);
    CHECK(v[1]->type() == ConversationType::GROUP);
    CHECK(v[0]->isParticipating(UserId("u2")));
    CHECK(!v[1]->isParticipating(UserId("u2")));
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
    CHECK(c.isParticipating(UserId("u1"))); CHECK(c.isParticipating(UserId("u2")));
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
    CHECK(!c.isParticipating(UserId("u1"))); CHECK(c.isParticipating(UserId("u2")));
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
    CHECK(m.status() == MessageStatus::DRAFTED);
    CHECK(!m.sentAt().has_value());
    CHECK(m.id() == MessageId("m1"));
    CHECK(m.conversationId() == ConversationId("c1"));
    CHECK(m.senderId() == UserId("u1"));
    CHECK(m.content().value() == "Hello");
    CHECK(m.priority() == Priority::NORMAL);
    CHECK(!m.hasAttachments());
}
void test_Message_FullLifecycle() {
    Message m = makeMessage();
    m.markAsBuffered();   CHECK(m.status() == MessageStatus::BUFFERED);
    m.markAsSending();   CHECK(m.status() == MessageStatus::SENDING);
    m.markAsSent();      CHECK(m.status() == MessageStatus::SENT);
    m.markAsDelivered(); CHECK(m.status() == MessageStatus::DELIVERED);
    m.markAsRead();      CHECK(m.status() == MessageStatus::READ);
}
void test_Message_MarkAsSentSetsSentAt() {
    Message m = makeMessage();
    m.markAsBuffered();
    m.markAsSending();
    CHECK(!m.sentAt().has_value());
    m.markAsSent();
    CHECK(m.sentAt().has_value());
}
void test_Message_FailedMarkAsSent_NoSentAt() {
    Message m = makeMessage();                       // DRAFTED: markAsSent is illegal
    CHECK_THROWS(m.markAsSent(), std::logic_error);
    CHECK(!m.sentAt().has_value());
    CHECK(m.status() == MessageStatus::DRAFTED);
}
void test_Message_UndeliveredFromSendingOrSent() {
    CHECK_NOTHROW(makeMessageIn(MessageStatus::SENDING).markAsUndelivered());
    CHECK_NOTHROW(makeMessageIn(MessageStatus::SENT).markAsUndelivered());
    Message m = makeMessageIn(MessageStatus::SENDING); m.markAsUndelivered();
    CHECK(m.status() == MessageStatus::UNDELIVERED);
}
void test_Message_Lifecycle_UndeliveredRetry() {
        Message m = makeMessage();
        m.markAsBuffered();   CHECK(m.status() == MessageStatus::BUFFERED);
        m.markAsSending();   CHECK(m.status() == MessageStatus::SENDING);
        m.markAsSent();      CHECK(m.status() == MessageStatus::SENT);
        m.markAsUndelivered(); CHECK(m.status() == MessageStatus::UNDELIVERED);
    }
void test_Message_EditContentOnlyIfDrafted() {
    Message m = makeMessage();
    m.editContent(MessageContent("Updated"));
    CHECK(m.content().value() == "Updated");
    m.markAsBuffered();
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
    Message m = makeMessage();
    m.addAttachment(AttachmentId("a1"));
    m.markAsBuffered();
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
    b.markAsBuffered();
    CHECK(a.status() == MessageStatus::DRAFTED);
    CHECK(b.status() == MessageStatus::BUFFERED);
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

    RUN_TEST(test_UserLogin_InternalWhitespaceIsRejected); RUN_TEST(test_UserLogin_TrimsWhitespace);
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
    RUN_TEST(test_Message_StartsBuffered); RUN_TEST(test_Message_FullLifecycle); RUN_TEST(test_Message_Lifecycle_UndeliveredRetry);
    RUN_TEST(test_User_Construction); RUN_TEST(test_User_InitialPresenceCanBeSet);
    RUN_TEST(test_User_IsOfflineIsConsistent); test_User_AwayOnlyFromOnline(); RUN_TEST(test_User_RoleChange);

    RUN_TEST(test_DirectConversation_Construction); RUN_TEST(test_DirectConversation_SelfChatIsAllowed);
    RUN_TEST(test_DirectConversation_SamePairIsOrderInsensitive); RUN_TEST(test_Conversation_PolymorphicUse);
    RUN_TEST(test_GroupConversation_Construction); RUN_TEST(test_GroupConversation_RejectsEmptyTitle);
    RUN_TEST(test_GroupConversation_AddParticipant); RUN_TEST(test_GroupConversation_RejectsDuplicateParticipant);
    RUN_TEST(test_GroupConversation_RemoveParticipant); RUN_TEST(test_GroupConversation_RejectsRemovingUnknownParticipant);
    RUN_TEST(test_Message_MarkAsSentSetsSentAt); RUN_TEST(test_Message_FailedMarkAsSent_NoSentAt);
    RUN_TEST(test_Message_UndeliveredFromSendingOrSent);
    RUN_TEST(test_Message_EditContentOnlyIfDrafted); RUN_TEST(test_Message_AddAndRemoveAttachment);
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
