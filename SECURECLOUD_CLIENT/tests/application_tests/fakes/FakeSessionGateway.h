#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "application/ISessionGateway.h"

/// @brief Hand-written fake of the port ISessionGateway (no network, no Qt).
/// @details Returns what the test told it to return. It counts its calls and remembers
///          what it received, so a test can check "0 calls" or "the login was normalized".
///
class FakeSessionGateway final : public application::ISessionGateway {

public:
    application::AuthResult login(const application::LoginRequestDTO& request) override
    {
        ++m_calls;
        m_lastLogin    = request.login.value();
        m_lastPassword = request.password;
        // simulate a throw runtime_error
        if (m_throws) { throw std::runtime_error("fake gateway: technical failure"); }
        return m_next;
    }

    void willReturn(application::AuthResult result)
    { m_next = std::move(result); }

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
    application::AuthResult m_next = application::AuthResult::failure(application::AuthError::SERVER_UNREACHABLE);
    int         m_calls = 0;
    bool        m_throws = false;
    std::string m_lastLogin;
    std::string m_lastPassword;
};
