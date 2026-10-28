#pragma once

#include <string>
#include <stdexcept>
#include <functional> // For std::hash

namespace domain {

/// @brief Unique identifier for a Conversation.
/// Immutable. Validated at construction. Cannot be empty.
class ConversationId final {
private:
    std::string m_value;

public:
    /// @brief Constructs a ConversationId from a non-empty string.
    /// @throws std::invalid_argument if value is empty.
    explicit ConversationId(const std::string& value) : m_value(value) {
        if (value.empty()) {
            throw std::invalid_argument("ConversationId cannot be empty");
        }
    }

    const std::string& value() const noexcept
    { return m_value; }

    bool operator==(const ConversationId& other) const noexcept
    { return m_value == other.m_value; }

    bool operator!=(const ConversationId& other) const noexcept
    { return !(m_value == other.m_value); }

};

} // namespace domain

namespace std {
/// @brief Hash specialization for domain::ConversationId.
template <>
struct hash<domain::ConversationId> {
    std::size_t operator()(const domain::ConversationId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};
} // namespace std