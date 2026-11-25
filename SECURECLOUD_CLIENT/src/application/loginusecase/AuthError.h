#pragma once

#include <string>

/// @file AuthError.h
/// @brief Failure families of the login scenario (UC-01), shared by Application, Infrastructure and UI.

namespace application {

/// @brief The reason why a login attempt failed. A failure is an expected result, not an exception.
/// @details The UI translates each value into a message (see UC1_Connexion.md section 7).
///          "Unknown login" and "wrong password" have the SAME value (INVALID_CREDENTIALS):
///          the Client cannot tell them apart, so it cannot tell which account exists.
enum class AuthError {
    INVALID_INPUT,        ///< Local validation failed, or the server answered 400 or 413. Bad Request.
    INVALID_CREDENTIALS,  ///< Unknown login or wrong password (HTTP 401). Unauthaurized.
    ACCOUNT_DISABLED,     ///< Credentials are correct but the account is disabled (HTTP 403). Forbidden.
    RATE_LIMITED,         ///< Too many attempts (HTTP 429), see retryAfterSeconds.Too Many Request (RFC 6585)
    SERVER_UNREACHABLE,   ///< No HTTP status at all: DNS, TCP, TLS, timeout, connection cut.
    SERVICE_UNAVAILABLE,  ///< Server side failure (HTTP 500, 502, 503, 504). An internal Server Error.
    INVALID_RESPONSE      ///< Unreadable or unexpected answer (bad JSON, unknown role...).
};

/// @brief Stable text for logs and test reports. NOT a message for the user (the UI may have those).
/// @param error the error code.
/// @return the error name. (see HTTP RFC 2616... 6585)
[[nodiscard]]
constexpr const std::string toString(AuthError error) noexcept
{
    switch (error) {
    case AuthError::INVALID_INPUT:       return "INVALID_INPUT";
    case AuthError::INVALID_CREDENTIALS: return "INVALID_CREDENTIALS";
    case AuthError::ACCOUNT_DISABLED:    return "ACCOUNT_DISABLED";
    case AuthError::RATE_LIMITED:        return "RATE_LIMITED";
    case AuthError::SERVER_UNREACHABLE:  return "SERVER_UNREACHABLE";
    case AuthError::SERVICE_UNAVAILABLE: return "SERVICE_UNAVAILABLE";
    case AuthError::INVALID_RESPONSE:    return "INVALID_RESPONSE";
    }
    return "UNKNOWN";
}

} // namespace application
