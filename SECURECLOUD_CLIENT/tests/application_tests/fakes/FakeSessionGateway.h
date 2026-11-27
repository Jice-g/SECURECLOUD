#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "application/gateway/ISessionGateway.h"

/// @brief Hand-written fake of the port ISessionGateway (no network).
/// @details Answers IMMEDIATELY: login() emits loginFinished() before returning, with what the test
///          told it to return. It counts its calls and remembers what it received, so a test can check
///          "0 calls" or "the login was normalized".
///
class FakeSessionGateway final : public application::ISessionGateway {

public:
    void login(const application::LoginRequestDTO& request) override
    {
        ++m_calls;
        m_lastLogin    = request.login.value();
        m_lastPassword = request.password;
        // simulate a technical failure of the adapter
        if (m_throws) { throw std::runtime_error("fake gateway: technical failure"); }
        emit loginFinished(m_authResult);
    }

    void willReturn(application::AuthResult result)
    { m_authResult = std::move(result); }

    void willThrow()
    { m_throws = true; }

    [[nodiscard]]
    int callCount() const noexcept
    { return m_calls; }

    [[nodiscard]]
    const std::string& lastLogin() const noexcept
    { return m_lastLogin; }

    [[nodiscard]]
    const std::string& lastPassword() const noexcept
    { return m_lastPassword; }

private:
    application::AuthResult m_authResult = application::AuthResult::failure(application::AuthError::SERVER_UNREACHABLE);
    int         m_calls = 0;
    bool        m_throws = false;
    std::string m_lastLogin;
    std::string m_lastPassword;
};

/// @brief Fake that does NOT answer by itself: the test triggers the answer by hand with deliver().
/// @details Used to test "a login is running" (isBusy, second execute() ignored).
///
class DeferredSessionGateway final : public application::ISessionGateway {

public:
    void login(const application::LoginRequestDTO& request) override
    {
        ++m_calls;
        m_lastLogin = request.login.value();
    }

    /// Emits loginFinished() now, as a real adapter would do later.
    void deliver(application::AuthResult result)
    { emit loginFinished(result); }

    [[nodiscard]]
    int callCount() const noexcept
    { return m_calls; }

    [[nodiscard]]
    const std::string& lastLogin() const noexcept
    { return m_lastLogin; }

private:
    int         m_calls = 0;
    std::string m_lastLogin;
};
