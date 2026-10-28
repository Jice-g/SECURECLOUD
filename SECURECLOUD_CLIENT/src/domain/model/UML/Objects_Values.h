#pragma once

#include <string>
#include <stdexcept>
 #include <functional>  // For std::hash

// Header-only.
//
/// @file   ObjectIds_Values.h
/// @brief  Value Objects representing unique identifiers in the Domain.
/// @note :
/// - The four identifiers (UserId, MessageId, ConversationId, AttachmentId) are  identical: a non-empty string.
/// - Written as separate classes to prevent confusion at compile time.
/// - A UserId can never be accidentally passed where a MessageId is expected.
/// - We use operators because we want to facilitate comparisons
namespace domain {

// =============================================================================
// UserId
// =============================================================================

/// @brief Represents a unique user identifier in the domain.
/// @throws std::invalid_argument if initialized with an empty string.
/// - Constructs from a non-empty string.
class UserId final {
	
 private:
    std::string m_value;

 public:
    explicit UserId(const std::string& value) : m_value(value) {
        if (value.empty()) {
            throw std::invalid_argument("UserId cannot be empty");
        }
    }
	
	[[nodiscard]]
    const std::string& value() const noexcept {
		return m_value;
		}

    bool operator==(const UserId& other) const noexcept {
		return m_value == other.m_value;
		}

    bool operator!=(const UserId& other) const noexcept {
		return !(m_value == other.m_value); }
};



// =============================================================================
// MessageId
// =============================================================================

/// @brief Unique identifier for a Message.
/// @throws std::invalid_argument if initialized with an empty string.
/// - Constructs from a non-empty string.
class MessageId final {
	
 private:
    std::string m_value;

 public:

    explicit MessageId(const std::string& value) : m_value(value) {
        if (value.empty()) {
            throw std::invalid_argument("MessageId cannot be empty");
        }
    }
	
	[[nodiscard]]
    const std::string& value() const noexcept {
		return m_value;
		}

    bool operator==(const MessageId& other) const noexcept {
		return m_value == other.m_value;
		}

    bool operator!=(const MessageId& other) const noexcept {
		return !(*this == other);
		}
};


// =============================================================================
// ConversationId
// =============================================================================

/// @brief Unique identifier for a Conversation.
/// @throws std::invalid_argument if initialized with an empty string.
/// - Constructs from a non-empty string.
class ConversationId final {
 private:
    std::string m_value;

 public:

    explicit ConversationId(const std::string& value) : m_value(value) {
        if (value.empty()) {
            throw std::invalid_argument("ConversationId cannot be empty");
        }
    }

	[[nodiscard]]
    const std::string& value() const noexcept { return m_value; }


    bool operator==(const ConversationId& other) const noexcept {
		return m_value == other.m_value;
		}

    bool operator!=(const ConversationId& other) const noexcept {
		return !(*this == other);
		}
};



// =============================================================================
// AttachmentId
// =============================================================================


/// @brief Unique identifier for an Attachment.
/// Cannot be empty.
///
class AttachmentId final {
private:
    std::string m_value;

public:

    explicit AttachmentId(const std::string& value) : m_value(value) {
        if (value.empty()) {
            throw std::invalid_argument("AttachmentId cannot be empty");
        }
    }

    const std::string& value() const noexcept { return m_value; }


    bool operator==(const AttachmentId& other) const noexcept {
		return m_value == other.m_value;
		}

    bool operator!=(const AttachmentId& other) const noexcept {
		return !(*this == other);
		}
};

} // namespace domain


// =============================================================================
// std::hash specializations - with <functional>
// =============================================================================
//
// This specialization allows the ID type to be used as key in unordered_map and unordered_set. 
// Example :
//     std::unordered_map<domain::UserId, domain::User> usersById;

namespace std {

/// @brief Hash specialization for domain::UserId.
template <>
struct hash<domain::UserId> {
    std::size_t operator()(const domain::UserId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};

/// @brief Hash specialization for domain::MessageId.
template <>
struct hash<domain::MessageId> {
    std::size_t operator()(const domain::MessageId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};

/// @brief Hash specialization for domain::ConversationId.
template <>
struct hash<domain::ConversationId> {
    std::size_t operator()(const domain::ConversationId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};

/// @brief Hash specialization for domain::AttachmentId.
template <>
struct hash<domain::AttachmentId> {
    std::size_t operator()(const domain::AttachmentId& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};

} // namespace std
***/