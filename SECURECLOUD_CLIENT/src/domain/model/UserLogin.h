
#include <string>
#include <string_view>
#include <stdexcept>

#include "domain/model/Enums.h"

/// @file   UserLogin.h
/// @brief  Value Object representing a user Login identifier.
///
/// A UserLogin is a normalized and validated identifier used for authentication.
/// It is not the display name: the UserLogin is technical and case-sensitive, 
/// the "display name" is what the user sees.
/// It will be easier to control the UserLogin format and avoid confusion if we enforce
/// a strict format now, and not in the Application layer or elsewhere.
///
/// Normalization happens when the UserLogin is set or reset:
///   - Whitespace is trimmed.
///   - All characters are converted to lowercase.
///   - Only alphanumeric characters, '_', '-', and '.' are allowed.
///
/// Validation happens after normalization:
///   - Length must be between 3 and 32 characters (for instance), it's better to define that here.
///   - Only alphanumeric characters, '_', '-', and '.' are allowed (we won't accept email address as UserLogin for now).
/// Header-only. No .cpp file required.

namespace domain {

/// @brief Value Object representing a normalized user Login.
/// @details Normalized to lowercase and allowed characters :
///  - UserLogin may be set or reset.
/// No one should inherit from this class to avoid breaking the invariants (==> final).
/// 
class UserLogin final {
	
 private:

	 std::string m_value; /// formally it's just a string, but we will enforce the invariants in the constructor.

    /// @brief Static constants 
	/// @details Minimum and maximum length for a valid UserLogin.
	/// must be defined in the class to be used in the constructor.
    /// 
    static constexpr std::size_t MIN_LENGTH = 3;
    static constexpr std::size_t MAX_LENGTH = 32;
	static constexpr std::string_view ALLOWED_CHARS = "abcdefghijklmnopqrstuvwxyz0123456789_-.";


    /// @brief clean whitespace from the string.
    /// @param s the string to trim.
    /// @return a new string without whitespace inside
    /// 
    static std::string cleanws(const std::string& s);


    /// @brief Checks whether a character is allowed in a UserLogin.
    /// @param c the character to check.
    /// @return true if the character is alphanumeric, '_', '-', or '.'.
    /// 
    static bool isAllowedChar(char c);

   
	/// @brief Converts a string to lowercase.
	/// @param s the string to convert.
	/// @returns a new string with all characters in lowercase.
    /// 
    static std::string toLower(const std::string& s);


	/// <summary>
	/// @brief Validates the normalized UserLogin string.
	/// @throws std::invalid_argument if the string is too short, too long, or contains disallowed characters.
	/// @param s the string to validate.
    /// 
    static void validate(const std::string& s);


public:

    /// @brief Constructs a UserLogin from a raw string.
    /// The input is trimmed and lowercased before validation.
    /// @param input: ogin string (may contain whitespace).
    /// @throws std::invalid_argument if the normalized value is too short,
    ///         too long, or contains disallowed characters.
    ///
    explicit UserLogin(const std::string& input)
        : m_value(std::move(toLower(cleanws(input))))
    {
        // Notice : If the value is invalid, an exception is thrown and the object is not constructed.
        // try .. catch instruction is necessary to catch the exception and rethrow it when UserLogin is set by construction or assignment.
        validate(m_value);
    }



    /// @brief Returns the normalized UserLogin value (lowercase, trimmed).
    /// @return A const reference to the internal string.
    /// 
    const std::string& value() const noexcept;

    /// @brief Equality comparison based on the normalized value.
    bool operator==(const UserLogin& compare) const noexcept;

    /// @brief Inequality comparison based on the normalized value.
    bool operator!=(const UserLogin& compare) const noexcept;

    /// @brief Resets a UserLogin from an input string.
    /// The input is trimmed and lowercased before validation.
    /// @param input : UserLogin string (may contain whitespace).
    /// @throws std::invalid_argument if the normalized value is too short,
    ///         too long, or contains disallowed characters.
    /// 
    UserLogin& operator= (const std::string& input);
    // Notice : If the value is invalid, an exception is thrown and the login value is left unchanged.
	// try .. catch instruction is necessary to catch the exception and notify the caller that the UserLogin was not changed.

};

} // namespace domain