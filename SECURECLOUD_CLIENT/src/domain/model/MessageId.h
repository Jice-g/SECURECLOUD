#pragma once

#include <string>
#include <stdexcept>
#include <functional> // For std::hash

namespace domain {

/// @brief Unique identifier for a Message.
/// Immutable. Validated at construction. Cannot be empty.
///
class MessageId final {
	
private:
    std::string m_value;

public:
    /// @brief Constructs a MessageId from a non-empty string.
    /// @throws std::invalid_argument if value is empty.
    explicit MessageId(const std::string& value) : m_value(value) {
        if (value.empty()) {
            throw std::invalid_argument("MessageId cannot be empty");
        }
    }

    const std::string& value() const noexcept
    { return m_value; }
    
    bool operator==(const MessageId& other) const noexcept
    { return m_value == other.m_value; }
    
    bool operator!=(const MessageId& other) const noexcept
    { return !(m_value == other.m_value); }
    
};

} // namespace domain

namespace std {
/// @brief Hash specialization for domain::MessageId.
template <>
struct hash<domain::MessageId> {
    std::size_t operator()(const domain::MessageId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};
} // namespace std