#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <cctype>     // For std::tolower, std::isspace, std::isalnum
#include <utility>    // For std::move

/// @file   Login.h
/// @brief  Value Object representing a user login identifier.
///
/// A Login is a normalized and validated identifier used for authentication.
/// It is not the display name: the Login is technical and case-sensitive, 
/// the "display name" is what the user sees.
/// It will be easier to control the login format and avoid confusion if we enforce
/// a strict format now, and not in the Application layer or elsewhere.
///
/// Normalization happens when the login is set or reset:
///   - Whitespace is trimmed.
///   - All characters are converted to lowercase.
///   - Only alphanumeric characters, '_', '-', and '.' are allowed.
///
/// Validation happens after normalization:
///   - Length must be between 3 and 32 characters (for instance), it's better to define that here.
///   - Only alphanumeric characters, '_', '-', and '.' are allowed (we won't accept email address as login for now).
/// Header-only. No .cpp file required.

namespace domain {

/// @brief Value Object representing a normalized user login.
/// @details Normalized to lowercase and allowed characters :
///  - login may be set or reset.
/// No one should inherit from this class to avoid breaking the invariants (==> final).
class Login final {
	
 private:
	 std::string m_value; /// formally it's just a string, but we will enforce the invariants in the constructor.

    /// @brief Static constants 
	/// @details Minimum and maximum length for a valid login.
	/// must be defined in the class to be used in the constructor.
    static constexpr std::size_t MIN_LENGTH = 3;
    static constexpr std::size_t MAX_LENGTH = 32;
	static constexpr std::string_view ALLOWED_CHARS = "abcdefghijklmnopqrstuvwxyz0123456789_-.";


    /// @brief clean whitespace from the string.
    /// @param s the string to trim.
    /// @return a new string without whitespace inside
    static std::string cleanws(const std::string& s)
    {
        std::string result;

        for (std::size_t i = 0; i < s.size(); ++i) {
            if (!std::isspace(static_cast<unsigned char>(s[i]))) {
                result += s[i];
            }
        }

		return result;
    }


    /// @brief Checks whether a character is allowed in a Login.
    /// @param c the character to check.
    /// @return true if the character is alphanumeric, '_', '-', or '.'.
    static bool isAllowedChar(char c)
    {
		if (ALLOWED_CHARS.find(c) == std::string_view::npos) {
			return false;
		}
		return true; // allowed character
    }



    static std::string toLower(const std::string& s)
    {
        std::string result = s;

        for (std::size_t i = 0; i < result.size(); ++i) {
            result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
        }

        return result;
    }

	static void validate(const std::string& s)
	{
		if (s.length() < MIN_LENGTH) {
			throw std::invalid_argument("Login is too short (minimum 3 characters)");
		}
		if (s.length() > MAX_LENGTH) {
			throw std::invalid_argument("Login is too long (maximum 32 characters)");
		}
		for (char c : s) {
			if (!isAllowedChar(c)) {
				throw std::invalid_argument(
					std::string("Login contains an invalid character: '") + c + "'");
			}
		}
	}


public:
    /// @brief Constructs a Login from a raw string.
    /// The input is trimmed and lowercased before validation.
    /// @param input : login string (may contain whitespace).
    /// @throws std::invalid_argument if the normalized value is too short,
    ///         too long, or contains disallowed characters.
    explicit Login(const std::string& input) : m_value( toLower(cleanws(input)) )
    {

        // Notice : If the value is invalid, an exception is thrown and the object is not constructed.
        // try .. catch instructions are necessary to catch the exception and rethrow it when Login is set by construction or assignment.
		validate(m_value);
    }


    /// @brief Returns the normalized login value (lowercase, trimmed).
    /// @return A const reference to the internal string.
    const std::string& value() const noexcept { return m_value; }

    /// @brief Equality comparison based on the normalized value.
    bool operator==(const Login& compare) const noexcept { return m_value == compare.m_value; }

    /// @brief Inequality comparison based on the normalized value.
    bool operator!=(const Login& compare) const noexcept { return m_value != compare.m_value; }

    /// @brief Resets a Login from an input string.
    /// The input is trimmed and lowercased before validation.
    /// @param input : login string (may contain whitespace).
    /// @throws std::invalid_argument if the normalized value is too short,
    ///         too long, or contains disallowed characters.
    Login& operator= (const std::string& input)
    {
        std::string new_value = toLower(cleanws(input)));

        // If the value is invalid, validate() throws and m_value is left unchanged.
        // try .. catch instructions are necessary to catch the exception and rethrow it when Login is set by construction or assignment.
		validate(new_value);

        m_value = std::move(new_value);
        return *this;
    }

};

} // namespace domain