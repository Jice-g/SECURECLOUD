#include <chrono>
#include <limits>
#include <stdexcept>
#include <string>

#include "domain_test.h"

#include "domain/model/AccessToken.h"
#include "domain/model/Session.h"
#include "domain/services/SessionService.h"

using namespace domain;
using std::chrono::seconds;

// =============================================================================
// Helpers
// =============================================================================

/// @brief A fixed instant, so that the tests never depend on the system clock.
static TimeStamp at(long long secondsSinceEpoch)
{ return TimeStamp(std::chrono::system_clock::time_point{seconds{secondsSinceEpoch}}); }

static User makeUser(const std::string& id = "u1", OnlineStatus presence = OnlineStatus::OFFLINE)
{ return User(UserId(id), UserLogin("alice"), UserRole::STD_USER, at(0), presence); }

static Session makeSession(const std::string& userId = "u1", long long expiresAt = 1100)
{ return Session(UserId(userId), AccessToken("tok-123"), at(expiresAt)); }

// =============================================================================
// AccessToken
// =============================================================================

void test_AccessToken_ValidConstruction()
{ CHECK(AccessToken("abc.def.ghi").value() == "abc.def.ghi"); }

void test_AccessToken_RejectsEmpty()
{ CHECK_THROWS(AccessToken(""), std::invalid_argument); }

void test_AccessToken_IsNeverModified() {
    // No trim, no normalization: the token is kept exactly as received.
    CHECK(AccessToken(" a b ").value() == " a b ");
}

void test_AccessToken_CopyKeepsValue() {
    AccessToken a("secret");
    AccessToken b = a;
    CHECK(b.value() == "secret");
    CHECK(a.value() == "secret");
}

// =============================================================================
// Session
// =============================================================================

void test_Session_Construction() {
    Session s = makeSession("u7", 1100);
    CHECK(s.userId() == UserId("u7"));
    CHECK(s.accessToken().value() == "tok-123");
    CHECK(s.expiresAt() == at(1100));
    CHECK(!s.isRevoked());
}

void test_Session_ActiveBeforeExpiry() {
    Session s = makeSession("u1", 1100);
    CHECK(s.isActive(at(1000)));
    CHECK(s.isActive(at(1099)));
    CHECK(!s.isExpired(at(1099)));
}

void test_Session_ExpiredFromExpiryInstant() {
    Session s = makeSession("u1", 1100);
    CHECK(s.isExpired(at(1100)));     // boundary: expired AT expiresAt
    CHECK(!s.isActive(at(1100)));
    CHECK(!s.isActive(at(5000)));
    CHECK(s.isExpired(at(5000)));
}

void test_Session_RevokedIsNotActive() {
    Session s = makeSession("u1", 1100);
    s.revoke();
    CHECK(s.isRevoked());
    CHECK(!s.isActive(at(1000)));     // still before expiry, but revoked
    CHECK(!s.isExpired(at(1000)));    // revoked is not the same thing as expired
}

void test_Session_RevocationIsDefinitive() {
    Session s = makeSession("u1", 1100);
    s.revoke();
    s.revoke();                       // twice: harmless
    CHECK(s.isRevoked());
    CHECK(!s.isActive(at(0)));        // whatever the time (even in the past), never active again
    CHECK(!s.isActive(at(1000)));
}

void test_Session_CopyIsIndependent() {
    Session a = makeSession("u1", 1100);
    Session b = a;
    b.revoke();
    CHECK(b.isRevoked());
    CHECK(!a.isRevoked());
    CHECK(a.isActive(at(1000)));
}

// =============================================================================
// SessionService
// =============================================================================

void test_SessionService_OpenPutsUserOnline() {
    User u = makeUser();
    CHECK(u.isOffline());
    Session s = SessionService::open(u, AccessToken("tok"), at(1000), seconds(900));
    CHECK(u.isOnline());
    CHECK(s.userId() == u.id());
    CHECK(s.accessToken().value() == "tok");
}

void test_SessionService_OpenComputesExpiry() {
    User u = makeUser();
    Session s = SessionService::open(u, AccessToken("tok"), at(1000), seconds(900));
    CHECK(s.expiresAt() == at(1900));
    CHECK(s.isActive(at(1000)));
    CHECK(s.isActive(at(1899)));
    CHECK(!s.isActive(at(1900)));
}

void test_SessionService_OpenFromAwayGoesOnline() {
    User u = makeUser("u1", OnlineStatus::AWAY);
    Session s = SessionService::open(u, AccessToken("tok"), at(1000), seconds(60));
    CHECK(u.presence() == OnlineStatus::ONLINE);
    CHECK(s.isActive(at(1000)));
}

void test_SessionService_OpenRefusesZeroLifetime() {
    User u = makeUser();
    CHECK_THROWS(SessionService::open(u, AccessToken("tok"), at(1000), seconds(0)), std::invalid_argument);
    CHECK(u.isOffline());             // the user is left untouched
}

void test_SessionService_OpenRefusesNegativeLifetime() {
    User u = makeUser();
    CHECK_THROWS(SessionService::open(u, AccessToken("tok"), at(1000), seconds(-1)), std::invalid_argument);
    CHECK(u.isOffline());
}

void test_SessionService_OpenRefusesOverflowingLifetime() {
    User u = makeUser();
    CHECK_THROWS(SessionService::open(u, AccessToken("tok"), at(1000),
                                      seconds(std::numeric_limits<long long>::max())),
                 std::invalid_argument);
    CHECK(u.isOffline());
}

void test_SessionService_CloseRevokesAndPutsOffline() {
    User u = makeUser();
    Session s = SessionService::open(u, AccessToken("tok"), at(1000), seconds(900));
    SessionService::close(u, s);
    CHECK(s.isRevoked());
    CHECK(!s.isActive(at(1000)));
    CHECK(u.isOffline());
}

void test_SessionService_CloseIsIdempotent() {
    User u = makeUser();
    Session s = SessionService::open(u, AccessToken("tok"), at(1000), seconds(900));
    SessionService::close(u, s);
    CHECK_NOTHROW(SessionService::close(u, s));
    CHECK(s.isRevoked());
    CHECK(u.isOffline());
}

void test_SessionService_CloseRefusesForeignSession() {
    User alice = makeUser("u1");
    User bob   = makeUser("u2");
    Session sAlice = SessionService::open(alice, AccessToken("tok"), at(1000), seconds(900));
    bob.onLine();
    CHECK_THROWS(SessionService::close(bob, sAlice), std::invalid_argument);
    CHECK(!sAlice.isRevoked());       // nothing changed
    CHECK(alice.isOnline());
    CHECK(bob.isOnline());
}

void test_SessionService_ReopenCreatesNewActiveSession() {
    User u = makeUser();
    Session first = SessionService::open(u, AccessToken("tok1"), at(1000), seconds(900));
    SessionService::close(u, first);
    Session second = SessionService::open(u, AccessToken("tok2"), at(1200), seconds(900));
    CHECK(u.isOnline());
    CHECK(second.isActive(at(1200)));
    CHECK(!first.isActive(at(1200)));   // the old one stays dead (BR-02)
}

// =============================================================================

int main(int argc, char** argv) {
    std::cout << "=====================================================\n"
              << "SecureCloud Client - Session Domain Test Suite (UC1 step 1)\n"
              << "=====================================================\n";

    RUN_TEST(test_AccessToken_ValidConstruction); RUN_TEST(test_AccessToken_RejectsEmpty);
    RUN_TEST(test_AccessToken_IsNeverModified); RUN_TEST(test_AccessToken_CopyKeepsValue);

    RUN_TEST(test_Session_Construction); RUN_TEST(test_Session_ActiveBeforeExpiry);
    RUN_TEST(test_Session_ExpiredFromExpiryInstant); RUN_TEST(test_Session_RevokedIsNotActive);
    RUN_TEST(test_Session_RevocationIsDefinitive); RUN_TEST(test_Session_CopyIsIndependent);

    RUN_TEST(test_SessionService_OpenPutsUserOnline); RUN_TEST(test_SessionService_OpenComputesExpiry);
    RUN_TEST(test_SessionService_OpenFromAwayGoesOnline);
    RUN_TEST(test_SessionService_OpenRefusesZeroLifetime); RUN_TEST(test_SessionService_OpenRefusesNegativeLifetime);
    RUN_TEST(test_SessionService_OpenRefusesOverflowingLifetime);
    RUN_TEST(test_SessionService_CloseRevokesAndPutsOffline); RUN_TEST(test_SessionService_CloseIsIdempotent);
    RUN_TEST(test_SessionService_CloseRefusesForeignSession);
    RUN_TEST(test_SessionService_ReopenCreatesNewActiveSession);

    std::cout << "\n=====================================================\n"
              << g_passedTests.size() << " passed, " << g_failedTests.size() << " failed, "
              << g_totalChecks << " checks.\n"
              << "=====================================================\n";

    const std::string reportPath = (argc > 1) ? argv[1] : "session_domain_report.txt";
    writeReport(reportPath, "Session Domain Tests Report");

    return g_failedTests.empty() ? 0 : 1;
}
