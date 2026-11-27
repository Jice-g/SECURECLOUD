// Application layer tests: AuthSession, AuthResult, AuthError, LoginUseCase (with FakeSessionGateway).
// No network, no event loop: the fakes answer synchronously (or by hand), so every signal is received
// before execute() returns. SessionManager has its own file (it also runs under the thread sanitizer).

#include <chrono>
#include <optional>
#include <string>

#include <vector>

#include <QObject>

#include "harness_test.h"
#include "application/loginusecase/AuthError.h"
#include "application/loginusecase/AuthResult.h"
#include "application/loginusecase/AuthSession.h"
#include "application/loginusecase/LoginUseCase.h"
#include "application/loginusecase/SessionManager.h"
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
// Helper: LoginUseCase is asynchronous. runLogin() connects to finished(), calls execute(),
// and returns the ONE outcome that was emitted (the fakes answer before execute() returns).
// =============================================================================

static LoginOutcome runLogin(LoginUseCase& useCase, const std::string& login, const std::string& password)
{
    std::vector<LoginOutcome> received;
    const QMetaObject::Connection link = QObject::connect(
        &useCase, &LoginUseCase::finished, [&received](const LoginOutcome& loc) { received.push_back(loc); });

    useCase.execute(login, password);
    QObject::disconnect(link);

    CHECK(received.size() == 1);  // exactly one finished() per accepted execute()
    return received.empty() ? LoginOutcome::failure(AuthError::INVALID_RESPONSE) : received.front();
}

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

    const LoginOutcome out = runLogin(useCase, "alice", "pw");

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

    const LoginOutcome out = runLogin(useCase, "  ALICE ", "pw");

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
    (void)runLogin(useCase, "alice", password);

    CHECK(gateway.lastPassword() == password);
}

static void test_Login_SecondSuccessReplacesTheSession()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);

    gateway.willReturn(successFor("alice", domain::UserRole::STD_USER, "t1"));
    CHECK(runLogin(useCase, "alice", "pw").ok());
    gateway.willReturn(successFor("bob", domain::UserRole::ADMIN, "t2"));
    CHECK(runLogin(useCase, "bob", "pw").ok());

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

    const LoginOutcome out = runLogin(useCase, rawLogin, password);

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
        (void)runLogin(useCase, "alice", customStringLength('x', 256));
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
        (void)runLogin(useCase, "alice", pw);
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

        const LoginOutcome out = runLogin(useCase, "alice", "pw");

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

    const LoginOutcome out = runLogin(useCase, "alice", "pw");

    CHECK(*out.error == AuthError::RATE_LIMITED);
    CHECK(out.retryAfterSeconds == 30);
}

static void test_Login_FailureDoesNotCloseAnOpenSession()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);

    gateway.willReturn(successFor("alice", domain::UserRole::STD_USER, "t1"));
    CHECK(runLogin(useCase, "alice", "pw").ok());

    gateway.willReturn(AuthResult::failure(AuthError::INVALID_CREDENTIALS));
    CHECK(!runLogin(useCase, "alice", "wrong").ok());
    CHECK(!runLogin(useCase, "", "pw").ok());  // local validation failure too

    CHECK(sessions.isAuthenticated());
    CHECK(sessions.accessToken().value() == "t1");
    CHECK(sessions.currentUser()->login().value() == "alice");
}

static void test_Login_TechnicalFailureOfThePortBecomesInvalidResponseAndFreesTheUseCase()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willThrow();

    const LoginOutcome out = runLogin(useCase, "alice", "pw");  // no exception escapes any more

    CHECK(!out.ok());
    CHECK(out.error.has_value() && *out.error == AuthError::INVALID_RESPONSE);
    CHECK(gateway.callCount() == 1);
    CHECK(!useCase.isBusy());
    CHECK(!sessions.isAuthenticated());
}

// =============================================================================
// LoginUseCase: asynchronous behaviour (isBusy, second execute ignored, stray answers)
// =============================================================================

static void test_Login_IsBusyBeforeDuringAndAfter()
{
    DeferredSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    std::vector<LoginOutcome> received;
    QObject::connect(&useCase, &LoginUseCase::finished, [&received](const LoginOutcome& o) { received.push_back(o); });

    CHECK(!useCase.isBusy());                       // before
    useCase.execute("alice", "pw");
    CHECK(useCase.isBusy());                        // during: the gateway has not answered yet
    CHECK(received.empty());
    CHECK(gateway.callCount() == 1);

    gateway.deliver(AuthResult::success(makeSession("alice")));

    CHECK(!useCase.isBusy());                       // after
    CHECK(received.size() == 1);
    CHECK(received[0].ok());
    CHECK(sessions.isAuthenticated());
}

static void test_Login_SecondExecuteWhileRunningIsIgnored()
{
    DeferredSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    std::vector<LoginOutcome> received;
    QObject::connect(&useCase, &LoginUseCase::finished, [&received](const LoginOutcome& o) { received.push_back(o); });

    useCase.execute("alice", "pw");
    useCase.execute("bob", "pw");          // ignored: no call to the port, no finished()
    useCase.execute("", "pw");             // ignored too, even if it would be INVALID_INPUT

    CHECK(gateway.callCount() == 1);
    CHECK(gateway.lastLogin() == "alice");
    CHECK(received.empty());

    gateway.deliver(AuthResult::success(makeSession("alice")));

    CHECK(received.size() == 1);           // one answer for one accepted execute()
    CHECK(received[0].user->login == "alice");
    CHECK(!useCase.isBusy());

    // and the use case is usable again
    useCase.execute("bob", "pw");
    CHECK(gateway.callCount() == 2);
    CHECK(useCase.isBusy());
}

static void test_Login_AnswerWithoutRunningLoginIsIgnored()
{
    DeferredSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    std::vector<LoginOutcome> received;
    QObject::connect(&useCase, &LoginUseCase::finished, [&received](const LoginOutcome& o) { received.push_back(o); });

    gateway.deliver(AuthResult::success(makeSession("alice")));  // nobody asked

    CHECK(received.empty());
    CHECK(!sessions.isAuthenticated());
    CHECK(!useCase.isBusy());
}

static void test_Login_InvalidInputDoesNotMakeTheUseCaseBusy()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);

    (void)runLogin(useCase, "", "pw");
    CHECK(!useCase.isBusy());
    (void)runLogin(useCase, "alice", "");
    CHECK(!useCase.isBusy());
    CHECK(gateway.callCount() == 0);
}

static void test_Login_SuccessOutcomeNeverCarriesAToken()
{
    FakeSessionGateway gateway;
    SessionManager sessions;
    LoginUseCase useCase(gateway, sessions);
    gateway.willReturn(successFor("alice", domain::UserRole::STD_USER, "secret-token-xyz"));

    const LoginOutcome out = runLogin(useCase, "alice", "pw");

    CHECK(out.ok());
    CHECK(out.user->login == "alice");     // LoginResultDTO has only login + role: no field can hold a token
    CHECK(sessions.accessToken().value() == "secret-token-xyz");  // the token stays in the Application
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
    RUN_TEST(test_Login_TechnicalFailureOfThePortBecomesInvalidResponseAndFreesTheUseCase);

    RUN_TEST(test_Login_IsBusyBeforeDuringAndAfter);
    RUN_TEST(test_Login_SecondExecuteWhileRunningIsIgnored);
    RUN_TEST(test_Login_AnswerWithoutRunningLoginIsIgnored);
    RUN_TEST(test_Login_InvalidInputDoesNotMakeTheUseCaseBusy);
    RUN_TEST(test_Login_SuccessOutcomeNeverCarriesAToken);


    std::cout << "\n==================================================================\n"
              << g_passedTests.size() << " passed, " << g_failedTests.size() << " failed, "
              << g_totalChecks << " checks.\n"
              << "==================================================================\n";

    const std::string reportPath = (argc > 1) ? argv[1] : "application_tests_report.txt";
    writeReport(reportPath, "Application tests (AuthResult, LoginUseCase)");

    return g_failedTests.empty() ? 0 : 1;   // non-zero exit code => CI fails
}
