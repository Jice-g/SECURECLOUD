#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "domain/model/ConversationId.h"
#include "domain/model/Message.h"
#include "domain/model/MessageId.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/Enums.h" // MessageStatus

/// @file   IMessageRepository.h
/// @brief  Domain contract: access to Messages.
///
/// All list results are ordered by createdAt ASCENDING (oldest first in the STACK), so the UI
/// can append them as they come. Implementations must guarantee it.

namespace domain {
	
	
class IMessageRepository {
	
public:

    virtual ~IMessageRepository() = default;

/* A revoir : associer toujours une Conversation Id ou bien appeler depuis une Conversation  */
    /// @brief Inserts or updates ("upsert" on MessageId): used for every state change.
    /// @throws RepositoryError on storage failure.
    virtual void save(const Message& message) = 0;

	/// @brief Find a Message in the storage by using Id
	/// @returns a Message value loaded or nothing if there is no match (optionnal)
    [[nodiscard]]
	virtual std::optional<Message> findById(const MessageId& id) const = 0;
	
	/* Limit of loading : nsample
	*   - it is not usefull to load all the messages from the database
	*   - the UI get only the most recent messages a set of `nsample` messages, and
	*     if the user scrolls back in time, another set of most recent messages are loaded
	*/

    /// @brief The `nsample` most recent messages of a Conversation.
	/// @parameters ConversationId, size_t (the size of a first sample)
	/// @notes finds the last message in the stack (that is a sample (a piece) of loaded messages)
    [[nodiscard]]
	virtual std::vector<Message> findLastMessages(const ConversationId& conversation, std::size_t nsample) const = 0;

    /// @brief Older page: up to `nsample` messages strictly before `before` time (scroll-back).
	/// @parameters ConversationId, size_t (the size of the next sample) = the nsample messages before
    [[nodiscard]]
	virtual std::vector<Message>  findBefore(const ConversationId& conversation, const TimeStamp& before, std::size_t nsample) const = 0;
	

    /// @brief Messages in a given state, all conversations.
    /// Use: the OUTBOX. On reconnection the Application asks for BUFFERED and
    /// UNDELIVERED messages and re-sends them (offline field backup, FILE-01).
    [[nodiscard]]
	virtual std::vector<Message> findByStatus(MessageStatus status) const = 0;

    /// @brief Local text search (F1.6).
	/// @note Server never searches content.Everything is encrypted in the server.
    [[nodiscard]]
	virtual std::vector<Message> searchText(const std::string& text, std::size_t limit) const = 0;  // F1.6
	
    virtual bool exists(const MessageId&) = 0;  // check duplication (UC-06)
	
    virtual void updateDeliveryStatus(const MessageId&, MessageStatus status) = 0;
	
};

} // namespace domain
