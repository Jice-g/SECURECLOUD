#pragma once

#include "domain/model/Enums.h"
#include "domain/model/UserLogin.h"
#include <cctype>

namespace domain {

    std::string UserLogin::cleanws(const std::string& s)
    {
        std::string result;
        result.reserve(s.size());   // one allocation, enough for the worst case

        for (size_t i = 0; i < s.size(); ++i)
        {
            if (std::isspace(static_cast<unsigned char>(s[i])) == 0) {
                result += s[i];
            }
        }

		return result;
    }


    bool UserLogin::isAllowedChar(char c)
    {
        if (UserLogin::ALLOWED_CHARS.find(c) == std::string_view::npos) {
			return false;
		}
		return true; // allowed character
    }


    std::string UserLogin::toLower(const std::string& s)
    {
        std::string result = s;

        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
        }

        return result;
    }


    void UserLogin::validate(const std::string& s)
	{
		if (s.length() < MIN_LENGTH) {
            throw std::invalid_argument("UserLogin is too short (minimum 3 characters)");
		}
		if (s.length() > MAX_LENGTH) {
            throw std::invalid_argument("UserLogin is too long (maximum 32 characters)");
		}
        for (char c : s)  {
            if (!UserLogin::isAllowedChar(c)) {
                throw std::invalid_argument(
                    std::string("UserLogin contains an invalid character: '") + c + "'");
            }
        }
		
	}

    /// @brief Returns the normalized UserLogin value (lowercase, trimmed).
    /// @return A const reference to the internal string.
    /// 
    const std::string& UserLogin::value() const noexcept
    { return m_value; }

    /// @brief Equality comparison based on the normalized value.
    bool UserLogin::operator==(const UserLogin& other) const noexcept
    { return m_value == other.m_value; }


    /// @brief Inequality comparison based on the normalized value.
    bool UserLogin::operator!=(const UserLogin& other) const noexcept
    { return m_value != other.m_value; }

    /// @brief Resets a UserLogin from an input string.
    /// The input is trimmed and lowercased before validation.
    /// @param input : UserLogin string (may contain whitespace).
    /// @throws invalid_argument if the normalized value is too short,
    ///         too long, or contains disallowed characters.
    /// 
    UserLogin& UserLogin::operator=(const std::string& input)
    {
        std::string new_value = toLower(cleanws(input));

        // If the value is invalid, validate() throws and m_value is left unchanged.
        // try .. catch instruction is ecessary to catch the exception and rethrow it when UserLogin is set by construction or assignment.
		validate(new_value);

        m_value = std::move(new_value);
        return *this;
    }

} // namespace domain