#include <utility>

#include "application/loginusecase/SessionManager.h"


namespace application {

void SessionManager::open(const AuthSession& session)
{
    // Snap : Build everything before locking:
    // if a copy throws (out of memory), the manager remains unchanged.
    SessionToken t = {session.accessToken(), session.refreshToken(), session.expiresAt(), session.user()};
    // our own copy: the caller's AuthSession stays untouched


    t.user.onLine();                       // BR-20: open session => ONLINE

    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessionToken = std::move(t);
}

void SessionManager::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessionToken.reset();
}

bool SessionManager::isAuthenticated() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessionToken.has_value();
}

std::optional<domain::User> SessionManager::currentUser() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessionToken->user;  // -> copy
}

std::optional<std::string> SessionManager::accessToken() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_sessionToken) {
        return std::nullopt; }
    return m_sessionToken->access;  // -> copy
}

} // namespace application
