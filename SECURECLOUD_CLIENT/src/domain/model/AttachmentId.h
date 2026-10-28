#pragma once

#include <string>
#include <stdexcept>
#include <functional>

namespace domain {

/// @brief Unique identifier for an Attachment.
/// Immutable. Validated at construction. Cannot be empty.
class AttachmentId final {
private:
    std::string m_value;

public:
    /// @brief Constructs an AttachmentId from a non-empty string.
    /// @param value The string value. Must not be empty.
    /// @throws std::invalid_argument if value is empty.
    explicit AttachmentId(const std::string& value) : m_value(value)
	{
        if (value.empty()) {
            throw std::invalid_argument("AttachmentId cannot be empty");
        }
    }

    /// @brief Returns the underlying string value.
    const std::string& value() const noexcept
	{ return m_value; }

    /// @brief Equality comparison based on the underlying value.
    bool operator==(const AttachmentId& other) const noexcept
	{ return m_value == other.m_value; }

    /// @brief Inequality comparison based on the underlying value.
    bool operator!=(const AttachmentId& other) const noexcept
	{ return !(m_value == other.m_value); }
};

} // namespace domain

namespace std {
/// @brief Hash specialization for domain::AttachmentId.
template <>
struct hash<domain::AttachmentId> {
    std::size_t operator()(const domain::AttachmentId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};
} // namespace std