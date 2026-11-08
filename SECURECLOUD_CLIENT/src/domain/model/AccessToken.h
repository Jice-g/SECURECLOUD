#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace domain {

/// @file AccessToken.h
/// @brief Value Object: the opaque access token given by the server after a login.
///
/// @details : a token cannot be empty - rule im^plemnted in the explicit constructor.
///
/// @warning A token is secret. No log, never writing it to disk, never show it in the UI.
///          then, the class has no setter, no operator<<, no comparison operator and no toString().
///
/// @note : The Client never reads inside the token
///  - a taken cannot be empty
///  - it is a JWT for the server, only an opaque string for the client: no decoding, no parsing, no inspection.
/// Header-only.
///
class AccessToken final {

private:

    std::string m_value;

public:

    /// @brief Constructs an AccessToken from a non-empty string.
    /// @param value The raw token, exactly as received from the server (never trimmed or modified).
    /// @throws std::invalid_argument if the string is empty (rule).
    ///
    explicit AccessToken(std::string value) : m_value(std::move(value))
    {
        if (m_value.empty()) {
            throw std::invalid_argument("AccessToken cannot be empty");
        }
    }

    /// @brief Gives the raw token, only to build the "Authorization" header of a request.
    /// @return A const reference to the internal string (const : the token remains immutable).
    ///
    [[nodiscard]]
    const std::string& value() const noexcept
    { return m_value; }
};

} // namespace domain
