// Application layer tests: AuthSession, AuthResult, AuthError, LoginUseCase (with FakeSessionGateway).
// No network, no Qt. SessionManager has its own file (it also runs under the thread sanitizer).

#include <chrono>
#include <optional>
#include <string>

#include "harness_test.h"
#include "application/AuthError.h"
#include "application/AuthResult.h"
#include "application/AuthSession.h"
#include "application/LoginUseCase.h"
#include "application/SessionManager.h"
#include "application_tests/fakes/FakeSessionGateway.h"

using namespace application;

// =============================================================================
// Helpers
// =============================================================================

static domain::TimeStamp fixedTimestamp()
{
    // used to test a fixed time (::now() would be unseizable)
    return domain::TimeStamp(std::chrono::system_clock::time_point{} + std::chrono::hours(1));
}

static domain::User makeUser(const std::string& login, domain::UserRole role = domain::UserRole::STD_USER)
{
    return domain::User(domain::UserId("u-" + login), domain::UserLogin(login), role, fixedTimestamp());
}

static AuthSession makeSession(const std::string& login = "alice",
                               domain::UserRole role = domain::UserRole::STD_USER,
                               const std::string& access = "access-1")
{
    // the access-token, a fixed expiry and a User
    return AuthSession(access, "refresh-1", fixedTimestamp(), makeUser(login, role));
}

static AuthResult successFor(const std::string& login = "alice",
                             domain::UserRole role = domain::UserRole::STD_USER,
                             const std::string& access = "access-1")
{
    return AuthResult::success(makeSession(login, role, access));
}

/// Builds a string of exactly `bytes` bytes.
/// cannot use 'std::string(bytes, c)' with the checkInvalidInput function :
/// erreur C2440: '<function-style-cast>' : impossible de convertir de 'initializer list' en 'std::string'
static std::string customStringLength(char c, std::size_t bytes)
{  return std::string(bytes, c); }

static const std::string E_ACUTE = "\xC3\xA9";  // "e acute" in UTF-8: 1 character, 2 bytes

static const AuthError ALL_ERRORS[] = {
    AuthError::INVALID_INPUT, AuthError::INVALID_CREDENTIALS, AuthError::ACCOUNT_DISABLED,
    AuthError::RATE_LIMITED, AuthError::SERVER_UNREACHABLE, AuthError::SERVICE_UNAVAILABLE,
    AuthError::INVALID_RESPONSE};

// =============================================================================
// AuthError / AuthSession / AuthResult
// =============================================================================

static void test_AuthError_ToStringIsDistinctAndNonEmpty()
{
    for (const AuthError a : ALL_ERRORS) {
        CHECK(std::string(toString(a)) != "UNKNOWN");
        for (const AuthError b : ALL_ERRORS) {
            if (a != b) { CHECK(std::string(toString(a)) != std::string(toString(b))); }
        }
    }
}

static void test_AuthSession_ExposesWhatItWasGiven()
{
    const AuthSession s = makeSession("alice", domain::UserRole::COORD, "tok");
    CHECK(s.accessToken() == "tok");
    CHECK(s.refreshToken() == "refresh-1");
    CHECK(s.expiresAt() == fixedTimestamp());
    CHECK(s.user().login().value() == "alice");
    CHECK(s.user().role() == domain::UserRole::COORD);
}

static void test_AuthSession_RejectsEmptyTokens()
{
    CHECK_THROWS(AuthSession("", "r", fixedTimestamp(), makeUser("alice")), std::invalid_argument);
    CHECK_THROWS(AuthSession("a", "", fixedTimestamp(), makeUser("alice")), std::invalid_argument);
    CHECK_NOTHROW(AuthSession("a", "r", fixedTimestamp(), makeUser("alice")));
}

static void test_AuthResult_Success()
{
    const AuthResult r = successFor("alice");
    CHECK(r.ok());
    CHECK(r.session().user().login().value() == "alice");
    CHECK_THROWS(r.error(), std::bad_optional_access);
}

static void test_AuthResult_Failure()
{
    const AuthResult r = AuthResult::failure(AuthError::RATE_LIMITED, 42);
    CHECK(!r.ok());
    CHECK(r.error() == AuthError::RATE_LIMITED);
    CHECK(r.retryAfterSeconds() == 42);
    CHECK_THROWS(r.session(), std::bad_optional_access);
}

static void test_AuthResult_FailureDefaultsAndClampsRetryAfter()
{
    CHECK(AuthResult::failure(AuthError::SERVER_UNREACHABLE).retryAfterSeconds() == 0);
    CHECK(AuthResult::failure(AuthError::RATE_LIMITED, -5).retryAfterSeconds() == 0);
}

// =============================================================================
// LoginUseCase: success
// =============================================================================

static void test_Login_SuccessOpensSessionAndReturnsLoginAndRole()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willReturn(successFor("alice", domain::UserRole::COORD, "secret-token"));

    const LoginOutcome out = useCase.execute("alice", "pw");

    CHECK(out.ok());
    CHECK(!out.error.has_value());
    CHECK(out.user->login == "alice");
    CHECK(out.user->role == domain::UserRole::COORD);
    CHECK(gateway.callCount() == 1);
    CHECK(sessions.isAuthenticated());
    CHECK(sessions.accessToken().value() == "secret-token");
    CHECK(sessions.currentUser().has_value());
    CHECK(sessions.currentUser()->isOnline());  // BR-20
}

static void test_Login_NormalizesLoginBeforeCallingThePort()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willReturn(successFor("alice"));

    const LoginOutcome out = useCase.execute("  ALICE ", "pw");

    CHECK(out.ok());
    CHECK(gateway.lastLogin() == "alice");
    CHECK(out.user->login == "alice");
}

static void test_Login_PasswordIsSentUnmodified()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willReturn(successFor("alice"));

    const std::string password = "  pass word  ";  // leading, trailing and inner spaces
    (void)useCase.execute("alice", password);

    CHECK(gateway.lastPassword() == password);
}

static void test_Login_SecondSuccessReplacesTheSession()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);

    gateway.willReturn(successFor("alice", domain::UserRole::STD_USER, "t1"));
    CHECK(useCase.execute("alice", "pw").ok());
    gateway.willReturn(successFor("bob", domain::UserRole::ADMIN, "t2"));
    CHECK(useCase.execute("bob", "pw").ok());

    CHECK(sessions.currentUser()->login().value() == "bob");
    CHECK(sessions.accessToken().value() == "t2");
}

// =============================================================================
// LoginUseCase: local validation (INVALID_INPUT, ZERO call to the port)
// =============================================================================

static void checkInvalidInput(const std::string& rawLogin, const std::string& password)
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);

    const LoginOutcome out = useCase.execute(rawLogin, password);

    CHECK(!out.ok());
    CHECK(out.error.has_value() && *out.error == AuthError::INVALID_INPUT);
    CHECK(gateway.callCount() == 0);
    CHECK(!sessions.isAuthenticated());
}

static void test_Login_BadLoginIsInvalidInputWithoutCall()
{
    checkInvalidInput("", "pw");
    checkInvalidInput("   ", "pw");
    checkInvalidInput("ab", "pw");                  // too short
    checkInvalidInput(customStringLength('a', 33), "pw");     // too long
    checkInvalidInput("al ice", "pw");              // inner space
    checkInvalidInput("alice@home", "pw");          // forbidden character
}

static void test_Login_BadPasswordIsInvalidInputWithoutCall()
{
    checkInvalidInput("alice", "");
    checkInvalidInput("alice", customStringLength('x', 257));
}

static void test_Login_PasswordLimitIsCountedInBytes()
{
    // Exactly 256 bytes: accepted (the fake answers a failure, but the port WAS called).
    {
        FakeSessionGateway gateway;
        SessionManager sessions;
        LoginUseCase useCase(gateway, sessions);
        (void)useCase.execute("alice", customStringLength('x', 256));
        CHECK(gateway.callCount() == 1);
    }
    // 128 x "e acute" = 128 characters but 256 bytes: accepted.
    {
        std::string pw;
        for (int i = 0; i < 128; ++i) { pw += E_ACUTE; }
        CHECK(pw.size() == 256);
        FakeSessionGateway gateway;
        SessionManager sessions;
        LoginUseCase useCase(gateway, sessions);
        (void)useCase.execute("alice", pw);
        CHECK(gateway.callCount() == 1);
    }
    // 129 x "e acute" = 129 characters but 258 bytes: rejected.
    {
        std::string pw;
        for (int i = 0; i < 129; ++i) { pw += E_ACUTE; }
        CHECK(pw.size() == 258);
        checkInvalidInput("alice", pw);
    }
}

// =============================================================================
// LoginUseCase: failures coming from the port
// =============================================================================

static void test_Login_EveryPortErrorIsPropagatedWithoutSession()
{
    for (const AuthError error : ALL_ERRORS) {
        FakeSessionGateway gateway;
        SessionManager sessions;
        LoginUseCase useCase(gateway, sessions);
        gateway.willReturn(AuthResult::failure(error));

        const LoginOutcome out = useCase.execute("alice", "pw");

        CHECK(!out.ok());
        CHECK(out.error.has_value() && *out.error == error);
        CHECK(out.retryAfterSeconds == 0);
        CHECK(gateway.callCount() == 1);
        CHECK(!sessions.isAuthenticated());
        CHECK(!sessions.currentUser().has_value());
    }
}

static void test_Login_RateLimitedKeepsItsDelay()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willReturn(AuthResult::failure(AuthError::RATE_LIMITED, 30));

    const LoginOutcome out = useCase.execute("alice", "pw");

    CHECK(*out.error == AuthError::RATE_LIMITED);
    CHECK(out.retryAfterSeconds == 30);
}

static void test_Login_FailureDoesNotCloseAnOpenSession()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);

    gateway.willReturn(successFor("alice", domain::UserRole::STD_USER, "t1"));
    CHECK(useCase.execute("alice", "pw").ok());

    gateway.willReturn(AuthResult::failure(AuthError::INVALID_CREDENTIALS));
    CHECK(!useCase.execute("alice", "wrong").ok());
    CHECK(!useCase.execute("", "pw").ok());  // local validation failure too

    CHECK(sessions.isAuthenticated());
    CHECK(sessions.accessToken().value() == "t1");
    CHECK(sessions.currentUser()->login().value() == "alice");
}

static void test_Login_TechnicalFailureOfThePortPropagatesAndLeavesSessionUntouched()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willThrow();

    CHECK_THROWS(useCase.execute("alice", "pw"), std::runtime_error);
    CHECK(!sessions.isAuthenticated());
}

// =============================================================================

int main(int argc, char* argv[])
{
    RUN_TEST(test_AuthError_ToStringIsDistinctAndNonEmpty);
    RUN_TEST(test_AuthSession_ExposesWhatItWasGiven);
    RUN_TEST(test_AuthSession_RejectsEmptyTokens);
    RUN_TEST(test_AuthResult_Success);
    RUN_TEST(test_AuthResult_Failure);
    RUN_TEST(test_AuthResult_FailureDefaultsAndClampsRetryAfter);

    RUN_TEST(test_Login_SuccessOpensSessionAndReturnsLoginAndRole);
    RUN_TEST(test_Login_NormalizesLoginBeforeCallingThePort);
    RUN_TEST(test_Login_PasswordIsSentUnmodified);
    RUN_TEST(test_Login_SecondSuccessReplacesTheSession);

    RUN_TEST(test_Login_BadLoginIsInvalidInputWithoutCall);
    RUN_TEST(test_Login_BadPasswordIsInvalidInputWithoutCall);
    RUN_TEST(test_Login_PasswordLimitIsCountedInBytes);

    RUN_TEST(test_Login_EveryPortErrorIsPropagatedWithoutSession);
    RUN_TEST(test_Login_RateLimitedKeepsItsDelay);
    RUN_TEST(test_Login_FailureDoesNotCloseAnOpenSession);
    RUN_TEST(test_Login_TechnicalFailureOfThePortPropagatesAndLeavesSessionUntouched);


    std::cout << "\n==================================================================\n"
              << g_passedTests.size() << " passed, " << g_failedTests.size() << " failed, "
              << g_totalChecks << " checks.\n"
              << "==================================================================\n";

    const std::string reportPath = (argc > 1) ? argv[1] : "application_tests_report.txt";
    writeReport(reportPath, "Application tests (AuthResult, LoginUseCase)");

    return g_failedTests.empty() ? 0 : 1;   // non-zero exit code => CI fails
}
