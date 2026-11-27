#pragma once

#include <memory>
#include <vector>

#include "domain/model/Conversation.h"
#include "domain/model/ConversationId.h"
#include "domain/model/UserId.h"

/// @file   IConversationRepository.h
/// @brief  Domain contract: access to Conversations (direct and group).
///
/// Conversation is polymorphic and abstract, so it cannot be returned by value
/// (slicing / abstract type): we return std::unique_ptr<Conversation>.
/// A null pointer means "not found".
///
/// Messages are NOT loaded with the conversation: they live behind
/// IMessageRepository (Message only holds a ConversationId).

namespace domain {

class IConversationRepository {
public:
    virtual ~IConversationRepository() = default;

    /// @brief Inserts or updates (upsert). The concrete type is recovered via type().
    /// @throws RepositoryError on storage failure.
    virtual void save(const Conversation& conversation) = 0;

	/// @brief Find a Conversation in the storage by using Id
	/// @returns a pointer on a Conversation loaded or nullptr if there is no match
    [[nodiscard]]
	virtual std::unique_ptr<Conversation>  findById(const ConversationId& id) const = 0;

    /// @brief The DIRECT conversation between two users, whatever the order.
    /// This is the lookup behind the rule "one direct conversation per pair".
    /// @return nullptr if the pair never talked yet.
    [[nodiscard]]
	virtual std::unique_ptr<Conversation>  findDirectBetween(const UserId& a, const UserId& b) const = 0;

    /// @brief Every conversation (direct + group) the user takes part in.
    [[nodiscard]]
	virtual std::vector<std::unique_ptr<Conversation>>  findByParticipant(const UserId& user) const = 0;
};

} // namespace domain
