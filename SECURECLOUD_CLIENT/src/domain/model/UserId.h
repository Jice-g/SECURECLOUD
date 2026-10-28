#pragma once

#include <string>
#include <stdexcept>
#include <functional>

#include "domain/model/Enums.h"

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

/*************************************************************************
*  UserId
**************************************************************************/


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


} // namespace

// =============================================================================
// std::hash specializations
// =============================================================================
//
// These specialization allows the ID type to be used as keys in std::unordered_map and std::unordered_set.
// We define them in `std` namespace (a rare but legal exception for user-defined types).
// See C++ tutorial and documentations

namespace std {

    /// @brief Hash specialization for domain::UserId.
    template <>
    struct hash<domain::UserId> {
        std::size_t operator()(const domain::UserId& id) const noexcept
        {
            return std::hash<std::string>{}(id.value());
        }
    };

} // namespace std