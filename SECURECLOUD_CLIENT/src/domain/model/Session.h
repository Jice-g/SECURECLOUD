#pragma once

#include <utility>

#include "domain/model/AccessToken.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/UserId.h"

namespace domain {

// BR means BusinessRules defined in the docs/
// Header Only because the behavior is not his business (will be used by the SessionService and the application layer)
/// @file Session.h
/// @brief Entity: the period during which a user is recognized as authenticated (docs : BR-01, BR-02).
///
/// A Session belongs to the current unique user until it expires or is revoked.
/// A revoked session NEVER becomes active again (BR-02):
/// there is no method to undo revoke()! To connect again, a new Session must be created.
///
/// The Session does not know the current time: the time is always given as a parameter (DOM-05).
/// Rule of a single entity => method of the entity (DOM-06). Included into the domain.
/// Header-only.
///
class Session final {

private:
/* Reminder :
 * The session is active as soon as this single entity has reveived the token.
 * The token belongs to exactly one unique user (the current user of the client).
 * The AccessToken is a String that is never empty (Object-Value)
 * It expires after a determined time (see the docs)
 * It may be revoked.
 * Therefor, the members are :
 *
*/
    UserId      m_userId;
    AccessToken m_token;
    TimeStamp   m_expiresAt;
    bool        m_revoked = false;

public:

    /// @brief Constructs an active (not revoked) session.
    /// @param userId    The user who owns the session.
    /// @param token     The access token received from the server.
    /// @param expiresAt The instant from which the session is expired.
    ///
    Session(UserId userId, AccessToken token, TimeStamp expiresAt)
        : m_userId(std::move(userId)), m_token(std::move(token)), m_expiresAt(expiresAt)
    { }

    [[nodiscard]]
    const UserId& userId() const noexcept
    { return m_userId; }

    [[nodiscard]]
    const AccessToken& accessToken() const noexcept
    { return m_token; }

    [[nodiscard]]
    const TimeStamp& expiresAt() const noexcept
    { return m_expiresAt; }

    [[nodiscard]]
    bool isRevoked() const noexcept
    { return m_revoked; }

    /// @brief Tells whether the session is expired at the given instant.
    /// @param now The current time will be compared to m_expiresAt
    /// @return true if now is equal to or after expiresAt.
    [[nodiscard]]
    bool isExpired(const TimeStamp& now) const noexcept
    { return now >= m_expiresAt; }

    /// @brief Tells whether the session can be used at the given instant (BR-01).
    /// @param now The current time will be passed in isExpired(...)
    /// @return true if the session is not revoked and not expired.
    [[nodiscard]]
    bool isActive(const TimeStamp& now) const noexcept
    { return !m_revoked && !isExpired(now); }

    /// @brief Revokes the session. Definitive (BR-02), and safe to call twice.
    void revoke() noexcept
    { m_revoked = true; }

};

} // namespace domain
