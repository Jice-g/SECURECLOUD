// SessionManager tests, including a concurrency test meant for -fsanitize=thread.

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <type_traits>

#include "harness_test.h"
#include "application/AuthSession.h"
#include "application/SessionManager.h"

using namespace application;

static domain::TimeStamp fixedExpiry()
{
    return domain::TimeStamp(std::chrono::system_clock::time_point{} + std::chrono::hours(1));
}

static AuthSession makeSession(const std::string& login, const std::string& access,
                               domain::UserRole role = domain::UserRole::STD_USER)
{
    return AuthSession(access, "refresh-" + access, fixedExpiry(),
                       domain::User(domain::UserId("u-" + login), domain::UserLogin(login), role, fixedExpiry()));
}

static_assert(!std::is_copy_constructible_v<SessionManager>, "a SessionManager holds a mutex: not copyable");

static void test_SessionManager_IsClosedAtStart()
{
    SessionManager sessions;
    CHECK(!sessions.isAuthenticated());
    CHECK(!sessions.currentUser().has_value());
    CHECK(!sessions.accessToken().has_value());
}

static void test_SessionManager_OpenStoresTokenAndOnlineUser()
{
    SessionManager sessions;
    sessions.open(makeSession("alice", "t1", domain::UserRole::COORD));

    CHECK(sessions.isAuthenticated());
    CHECK(sessions.accessToken().value() == "t1");
    CHECK(sessions.currentUser().has_value());
    CHECK(sessions.currentUser()->login().value() == "alice");
    CHECK(sessions.currentUser()->role() == domain::UserRole::COORD);
    CHECK(sessions.currentUser()->isOnline());
}

static void test_SessionManager_OpenDoesNotModifyTheCallersSession()
{
    SessionManager sessions;
    const AuthSession grant = makeSession("alice", "t1");
    CHECK(grant.user().isOffline());

    sessions.open(grant);

    CHECK(grant.user().isOffline());          // we worked on our own copy
    CHECK(grant.accessToken() == "t1");
}

static void test_SessionManager_CloseForgetsEverything()
{
    SessionManager sessions;
    sessions.open(makeSession("alice", "t1"));
    sessions.close();

    CHECK(!sessions.isAuthenticated());
    CHECK(!sessions.currentUser().has_value());
    CHECK(!sessions.accessToken().has_value());
}

static void test_SessionManager_CloseOnClosedSessionIsHarmless()
{
    SessionManager sessions;
    CHECK_NOTHROW(sessions.close());
    CHECK(!sessions.isAuthenticated());
}

static void test_SessionManager_SecondOpenReplacesTheFirst()
{
    SessionManager sessions;
    sessions.open(makeSession("alice", "t1"));
    sessions.open(makeSession("bob", "t2", domain::UserRole::ADMIN));

    CHECK(sessions.accessToken().value() == "t2");
    CHECK(sessions.currentUser()->login().value() == "bob");
    CHECK(sessions.currentUser()->role() == domain::UserRole::ADMIN);
}

static void test_SessionManager_CurrentUserIsACopy()
{
    SessionManager sessions;
    sessions.open(makeSession("alice", "t1"));

    std::optional<domain::User> copy = sessions.currentUser();
    copy->offLine();                                   // changing the copy...

    CHECK(sessions.currentUser()->isOnline());         // ...does not change the manager
}

/// One thread opens/closes in a loop, another one reads. Without the mutex this is a data race:
/// run it with -fsanitize=thread to see the difference.
static void test_SessionManager_ConcurrentAccess()
{
    SessionManager sessions;
    std::atomic<bool> stop{false};
    std::atomic<bool> readerStarted{false};
    std::atomic<int> inconsistent{0};

    std::thread writer([&] {
        const AuthSession grant = makeSession("alice", "t1");
        while (!readerStarted) { std::this_thread::yield(); }   // make sure both threads really overlap
        for (int i = 0; i < 20000; ++i) {
            sessions.open(grant);
            sessions.close();
        }
        stop = true;
    });

    std::thread reader([&] {
        readerStarted = true;
        while (!stop) {
            const std::optional<domain::User> user = sessions.currentUser();
            if (user && !user->isOnline()) { ++inconsistent; }   // an open session always means ONLINE
            (void)sessions.isAuthenticated();
            (void)sessions.accessToken();
        }
    });

    writer.join();
    reader.join();

    CHECK(inconsistent == 0);
    CHECK(!sessions.isAuthenticated());
}

int main(int argc, char* argv[])
{
    RUN_TEST(test_SessionManager_IsClosedAtStart);
    RUN_TEST(test_SessionManager_OpenStoresTokenAndOnlineUser);
    RUN_TEST(test_SessionManager_OpenDoesNotModifyTheCallersSession);
    RUN_TEST(test_SessionManager_CloseForgetsEverything);
    RUN_TEST(test_SessionManager_CloseOnClosedSessionIsHarmless);
    RUN_TEST(test_SessionManager_SecondOpenReplacesTheFirst);
    RUN_TEST(test_SessionManager_CurrentUserIsACopy);
    RUN_TEST(test_SessionManager_ConcurrentAccess);

    if (argc > 1) { writeReport(argv[1], "SessionManager tests"); }
    return g_failedTests.empty() ? 0 : 1;
}
