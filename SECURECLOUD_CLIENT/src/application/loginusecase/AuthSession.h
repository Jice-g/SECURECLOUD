#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "domain/model/TimeStamp.h"
#include "domain/model/User.h"

/// @file AuthSession.h

namespace application {


struct SessionToken {
    std::string       access;
    std::string       refresh;  // kept in RAM, unused in v1 (ADR UC1-08)
    domain::TimeStamp expiresAt;
    domain::User user;
};

/// @brief What the server returns after a successful login: tokens, expiry and the user profile.
/// @details Tokens are OPAQUE strings: the Client never decodes them (they are JWT on the server side).
///          Like a value object, an AuthSession exists (see AuthResult) only if it is valid: both tokens are non-empty.
///          if in failed there is no open Session.
///
class AuthSession final {
public:
    /// @brief Builds a session grant.
    /// @param accessToken  short-lived token sent with each request. Must not be empty.
    /// @param refreshToken token used to renew the access token (kept in RAM, unused in v1). Must not be empty.
    /// @param expiresAt    instant at which the access token expires (computed with the CLIENT clock).
    /// @param user         the authenticated user's profile.
    /// @throws std::invalid_argument if a token is empty.
    ///
    AuthSession(std::string accessToken, std::string refreshToken, domain::TimeStamp expiresAt, domain::User user) :
        m_sessionToken{std::move(accessToken), std::move(refreshToken), expiresAt, std::move(user)}

    {
        if (m_sessionToken.access.empty()) {
            throw std::invalid_argument("AuthSession: access token is empty");
        }
        if (m_sessionToken.refresh.empty()) {
            throw std::invalid_argument("AuthSession: refresh token is empty");
        }
    }

    [[nodiscard]]
    const std::string& accessToken() const noexcept
    { return m_sessionToken.access; }

    [[nodiscard]]
    const std::string& refreshToken() const noexcept
    { return m_sessionToken.refresh; }

    [[nodiscard]]
    const domain::TimeStamp& expiresAt() const noexcept
    { return m_sessionToken.expiresAt; }

    [[nodiscard]]
    const domain::User& user() const noexcept
    { return m_sessionToken.user; }

private:
    SessionToken      m_sessionToken;
};

} // namespace application
