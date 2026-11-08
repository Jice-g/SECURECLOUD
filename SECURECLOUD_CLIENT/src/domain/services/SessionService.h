#pragma once

#include <chrono>
#include <stdexcept>
#include <utility>

#include "domain/model/AccessToken.h"
#include "domain/model/Session.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/User.h"

namespace domain {

/* Notes :
 * SessionService is "a service" for the application layer
 * It is not an entity object that is constructed to live in the application. No instanciation and stateless.
 * It creates a session with open(...) and is called to revoke it.
 * Therefor every methods are static.
 */

/// @file SessionService.h
/// @brief Domain service: ties the presence of a User to the life of its Session (BR-20) : Connected.
///
/// A rule that involves two entities (User and Session) is a domain service (DOM-06).
/// The service is pure: no state, no repository, no system clock (DOM-04, DOM-05). The time is a parameter.
/// Header-only.
///
class SessionService final {

public:

    SessionService() = delete;   // only static functions: nothing to instantiate.

    /// @brief Opens a session for a user and puts the user ONLINE.
    /// @param user     The authenticated user (set ONLINE, even if it was AWAY).
    /// @param token    The access token received from the server.
    /// @param now      The current time (injected).
    /// @param lifetime The validity duration of the token (in seconds); the session expires at now + lifetime.
    /// @return The new active Session.
    /// @throws std::invalid_argument if lifetime is zero or negative, or if now + lifetime cannot be represented.
    ///         In that case the user is left unchanged.
    ///
    [[nodiscard]]
    static Session open(User& user, AccessToken token, const TimeStamp& now, std::chrono::seconds lifetime)
    {
        // Check injected data :
        if (lifetime <= std::chrono::seconds::zero()) {
            throw std::invalid_argument("SessionService::open: lifetime must be positive");
        }
        const auto limitmax = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::time_point::max() - now.value());
        if (lifetime > limitmax) {
            throw std::invalid_argument("SessionService::open: lifetime is too large");
        }

        // if ok, create Session :
        Session session(user.id(), std::move(token), TimeStamp(now.value() + lifetime));
        user.onLine();   // secure : everything has been checked, nothing can fail after this line
        return session;
    }

    /// @brief Closes a session: revokes it (definitively) and puts the user OFFLINE.
    /// @param user    The user who owns the session.
    /// @param session The session to close. Closing an already closed session is harmless.
    /// @throws std::invalid_argument if the session does not belong to this user.
    ///         In that case neither the user nor the session is changed.
    ///
    static void close(User& user, Session& session)
    {
        // check user id mismatch :
        if (session.userId() != user.id()) {
            throw std::invalid_argument("SessionService::close: the session does not belong to this user");
        }
        session.revoke();
        user.offLine();
    }
};

} // namespace domain
