#pragma once

#include <mutex>
#include <optional>
#include <string>

#include "application/loginusecase/AuthSession.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/User.h"

/// @file SessionManager.h

namespace application {

/// @brief Holds the current session ("who is logged in, with which tokens?") in RAM only.
/// @details States: Closed (initial) --open()--> Open --close()--> Closed. open() on an Open session replaces it.
///          Invariants:
///          - tokens and current user are present TOGETHER or absent TOGETHER (one lock covers both);
///          - an open session means the current user is ONLINE (BR-20);
///          - accessors return COPIES, never references into the object (another thread may change it).
///          Thread safety: the login worker thread writes, the UI thread reads. Every method locks m_mutex.
///          Tokens are never logged and never written to disk.
///
class SessionManager final {

public:
    // default constructor and no copy (a mutex cannot be copied)
    SessionManager() = default;    
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    /// @brief Opens a session (or replaces the current one). Marks the user ONLINE.
    /// @param session the AutSession received from the server (left untouched).
    void open(const AuthSession& session);

    /// @brief Closes the session locally (UC-09): forgets tokens and user. No server call.
    void close();

    /// @brief true if a session is open. (Token expiry will comes with refresh, Roadmap.)
    [[nodiscard]]
    bool isAuthenticated() const;

    /// @brief A copy of the connected user, or nothing if the session is closed (optionnal).
    [[nodiscard]]
    std::optional<domain::User> currentUser() const;

    /// @brief A copy of the access token, or nothing if the session is closed.
    [[nodiscard]]
    std::optional<std::string> accessToken() const;

private:
    mutable std::mutex           m_mutex;  // mutable: locking is allowed inside const methods
    std::optional<SessionToken>  m_sessionToken;
};

} // namespace application
