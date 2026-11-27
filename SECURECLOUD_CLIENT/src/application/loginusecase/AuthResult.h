#pragma once

#include <optional>
#include <utility>

#include "application/loginusecase/AuthError.h"
#include "application/loginusecase/AuthSession.h"

/// @file AuthResult.h

namespace application {

/// @brief Return value 'AuthResult of the port ISessionGateway::login:
///        either a session (if it worked) OR an error (if it failed).
///        so both are <optionnal>, see constructor.
/// @details The constructor is private: only success() and failure() will build a result,
///          it is build by a static method.
///          so an inconsistent result cannot exist.
///          This is a plain value, not an exception: a wrong password is a normal outcome.
///
class AuthResult final {

public:
    /// @brief A successful login.
    /// @details the AuthError is optionnal. When it succeded there is no error message: nullopt
    ///
    [[nodiscard]]
    static AuthResult success(AuthSession session)
    { return AuthResult(std::move(session), std::nullopt, 0); }

    /// @brief A failed login.
    /// @param error the failure family.
    /// @param retryAfterSeconds delay before a new attempt (RATE_LIMITED); 0 if unknown. Negative values become 0.
    /// @details the AuthSession is also optionnal, because it is given only if le login sequence succeded
    [[nodiscard]]
    static AuthResult failure(AuthError error, int retryAfterSeconds = 0)
    {
        int retry = retryAfterSeconds < 0 ? 0 : retryAfterSeconds;
        return AuthResult(std::nullopt, error, retry);
    }

    /// @brief true if the login succeeded.
    [[nodiscard]]
    bool ok() const noexcept
    { return m_session.has_value(); }

    /// @throws std::bad_optional_access (nullopt has no value) if the login failed
    /// test bool ok() first, because calling it blindly is a bug.
    [[nodiscard]]
    const AuthSession& session() const
    { return m_session.value(); }

    /// @throws std::bad_optional_access (nullopt has no value) if the login succeeded.
    /// test bool !ok() first, because calling it blindly is a bug as well.
    [[nodiscard]]
    AuthError error()const
    { return m_error.value(); }

    [[nodiscard]]
    int retryAfterSeconds() const noexcept
    { return m_retryAfterSeconds; }

private:
    AuthResult(std::optional<AuthSession> session, std::optional<AuthError> error, int retryAfterSeconds)
        : m_session(std::move(session)), m_error(error), m_retryAfterSeconds(retryAfterSeconds)
    {}
     // session or ertror either one or the other (both optionnal)
    std::optional<AuthSession> m_session;
    std::optional<AuthError>   m_error;
    int  m_retryAfterSeconds;
};

} // namespace application
