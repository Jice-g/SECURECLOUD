#pragma once

#include <QByteArray>

#include "application/loginusecase/AuthResult.h"
#include "application/loginusecase/LoginProcessDTO.h"
#include "domain/model/TimeStamp.h"

/// @file AuthJsonMapper.h

namespace infrastructure {

/// @brief Translates between the wire format (HTTP status, JSON) and our objects, for the login (UC-01).
/// @details Stateless class, PURE static functions: no network, no QObject, no clock
///          (the current time is a parameter), so it is testable with hard-coded strings.
///          This is the ONLY place where the snake_case names of the contract (Contrat v0.1)
///          are known; everywhere else the code uses its own C++ conventions (ADR UC1-01).
///          Minimal version: one AuthError per HTTP status; a 200 with a missing, empty or badly typed
///          field, or an unknown role, becomes INVALID_RESPONSE.
///          Neither the password, nor the tokens, nor the body are ever logged here.
///
class AuthJsonMapper final {

public:
    AuthJsonMapper() = delete;  // only static functions

    /// @brief Builds the JSON body of POST /auth/login.
    /// @param request the normalized login and the raw password (sent unmodified, spaces included).
    /// @return UTF-8 JSON: {"login": "<normalized>", "password": "<raw>", "mfa_code": null}
    ///         (compact; keys in alphabetical order, which JSON allows).
    ///
    [[nodiscard]]
    static QByteArray buildLoginBody(const application::LoginRequestDTO& request);

    /// @brief Turns an HTTP answer into an AuthResult (session or AuthError).
    /// @param httpStatus        the HTTP status code (200, 401...).
    /// @param retryAfterSeconds already read by the gateway from Retry-After (0 if absent or unreadable).
    /// @param body              the raw body of the answer (may be empty or not JSON).
    /// @param now               the CLIENT clock: expiresAt = now + expires_in (no clock shift between machines).
    /// @return success only for status 200 with a usable body; otherwise an AuthError.
    ///         Never throws for a bad answer (a bad answer is a normal result, not an exception).
    ///
    [[nodiscard]]
    static application::AuthResult
    parseLoginResponse(int httpStatus, int retryAfterSeconds, const QByteArray& body,
                       const domain::TimeStamp& now);
};

} // namespace infrastructure
