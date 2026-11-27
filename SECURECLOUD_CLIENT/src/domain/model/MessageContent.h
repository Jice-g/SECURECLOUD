#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>  // For std::all_of
#include <cctype>     // For std::isspace



/// @file   MessageContent.h
/// @brief  Value Object representing the text content of a Message.
///
/// A MessageContent wraps a string and enforces:
///   - Maximum length: 4096 characters.
///   - No NUL ('\0') characters.
///   - Trailing whitespace is trimmed (leading is preserved for some custom formatting).
///
/// @note Empty content IS allowed at this level. 
///       BUT : The Message entity enforces the invariant "content OR attachment".
///       You may just send an attachment with no message content in addition.
///       This separation keeps the two concepts independent.
///
/// This file is header-only.
/// 
/// 

namespace domain {

    /// @brief Value Object representing the text content of a Message.
    /// Final : no inheritance allowed.
    /// Immutable. Validated at construction. Trailing whitespace trimmed.
    ///
    class MessageContent final
    {

    private:

        std::string m_text;

        /// @brief Maximum allowed length for a message content.
        static constexpr std::size_t MAX_BYTES = 4096;

        /// @brief Trims trailing whitespace only.
        /// @param input The string to trim.
        /// @return A new string without trailing whitespace.
        ///
        /// @note Leading whitespace is preserved to allow user-formatted
        ///       content such as bullet lists or code indentation or any other things.
        /// 
        static std::string trimTrailing(const std::string& input)
        {
            auto end = input.end(); 
            /* pedagogic insert :
             * *******************
             * WARNING : end() is an iterator : end() points to one position PAST the last character (input[size]!!) so conceptually on '\0'.
             * But if we try to read *input.end(), we get undefined behavior (often a crash). It is not a character. It is a boundary.
             * So we must use (end - 1) in the loop. ::begin() points exactly on the first one. 
             */

            while (end != input.begin() && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
                --end;
            }
            return std::string(input.begin(), end);
        }


    public:
        /// @brief Constructs a MessageContent from a string.
        ///
        /// Trailing whitespace is trimmed.
        /// The result may be empty for attachment-only messages.
        ///
        /// @param text The raw text content.
        /// @throws std::invalid_argument if text exceeds MAX_LENGTH
        ///         or contains a NUL character.
        ///
        explicit MessageContent(const std::string& text)
            : m_text(trimTrailing(text))
        {
            if (m_text.length() > MAX_BYTES) {
                throw std::invalid_argument("MessageContent exceeds maximum length (4096)");
            }
            if (m_text.find('\0') != std::string::npos) {
                throw std::invalid_argument("MessageContent cannot contain NUL characters");
            }
        }


        /// @brief Returns the normalized content (trailing whitespace trimmed).
        /// @return A const reference to the internal string.
        /// 
        const std::string& value() const noexcept
        { return m_text; }


        /// @brief Checks whether the content is empty or contains only whitespace.
        /// @return true if all characters are whitespace or the string is empty.
        ///
        /// @note Used by the Message entity to enforce "content OR attachment".
        /// 
        bool isEmpty() const noexcept
        {   
            // for all of them test the predicate wich is a lambda (function with no name [])
            return std::all_of(m_text.begin(), m_text.end(),
                [](unsigned char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }  );
        }


        /// @brief Returns the number of characters in the content.
        /// @return The length of the content, in bytes.
        std::size_t length() const noexcept
        { return m_text.length(); }


        /// @brief Equality comparison based on the raw content.
        bool operator==(const MessageContent& other) const noexcept {
            return m_text == other.m_text;
        }

        /// @brief Inequality comparison based on the raw content.
        bool operator!=(const MessageContent& other) const noexcept {
            return m_text != other.m_text;
        }
    };

} // namespace domain