#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <optional>

#include "domain/model/MessageId.h"
#include "domain/model/ConversationId.h"
#include "domain/model/UserId.h"
#include "domain/model/MessageContent.h"
#include "domain/model/AttachmentId.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/Enums.h"

/// @file   Message.h
/// @brief  Entity representing a single instant message.
///
/// A Message is a child entity of a Conversation.
/// It is created in BUFFERED state and follows a strict lifecycle.
/// It enforces the rule "content OR attachment".
///
/// This entity is part of the pure Domain layer and has no knowledge
/// of Qt, SQLite, or network protocols.
///

/* Note :
 * The Domain Rule is not "a message is sent to a recipient." It is "member of a Conversation."
 * The Message entity just holds the ConversationId and Attachment.
 * The routing logic belongs to the Application and Infrastructure layers, not the Domain entity
 */

namespace domain {

/// @brief  Entity representing a single instant message.
/// @note   it has got the senderId the recipent(s) is defined in the conversation
///         The routing logic belongs to the Application and Infrastructure layers
///         Format, encrypt, send with HTTP protocole
///
class Message  {

private:
    MessageId m_id;
    ConversationId m_conversationId;
    UserId m_senderId;
    MessageContent m_content;
    TimeStamp m_createdAt;
    std::optional<TimeStamp> m_sentAt;   // set by markAsSent()
    Priority m_priority;
    MessageStatus m_status;
    std::vector<AttachmentId> m_attachments;

public:

  /*  /// @brief Constructs a new Message in BUFFERED state.
    /// @param id The unique message identifier.
    /// @param conversationId The conversation this message belongs to.
    /// @param senderId The user who sent the message.
    /// @param content The text content (may be empty if an attachment is added).
    /// @param createdAt The creation timestamp.
    /// @param priority The priority level of the message.
    /// */

    Message(const MessageId& id,
            const ConversationId& conversationId,
            const UserId& senderId,
            const MessageContent& content,
            const TimeStamp& createdAt,
            Priority priority)
      : m_id(id),
        m_conversationId(conversationId),
        m_senderId(senderId),
        m_content(content),
        m_createdAt(createdAt),
        m_priority(priority)

    {
        m_status = MessageStatus::DRAFTED; // Always starts as DRAFTED

        // Note: We do not enforce "content OR attachment" here because
        // attachments might be added immediately after construction.
        // The Application layer MUST check it by calling isValid() before persisting.
        // The responsability to send a valid message is given to the application layer.
    }


    // --- Getters ---
    const MessageId& id() const noexcept
    { return m_id; }

    const ConversationId& conversationId() const noexcept
    { return m_conversationId; }

    const UserId& senderId() const noexcept
    { return m_senderId; }

    const MessageContent& content() const noexcept
    { return m_content; }

    const TimeStamp& createdAt() const noexcept
    { return m_createdAt; }

    const std::optional<TimeStamp>& sentAt() const noexcept
    { return m_sentAt; }

    Priority priority() const noexcept
    { return m_priority; }

    MessageStatus status() const noexcept
    { return m_status; }

    const std::vector<AttachmentId>& attachments() const noexcept
    { return m_attachments; }



    // --- State Machine ---

    /// @brief Transitions DRAFTED -> BUFFERED or UNDELIVERED -> BUFFERED requeue() (retry)
    /// @throws std::logic_error if current status is not DRAFTED.
    void markAsBuffered();

    /// @brief Transitions BUFFERED -> SENDING.
    /// @throws std::logic_error if current status is not BUFFERED.
    void markAsSending();

    /// @brief Transitions SENDING -> SENT.
    /// @throws std::logic_error if current status is not SENDING.
    /// @details set sentAt at the moment now()
    void markAsSent();

    /// @brief Transitions SENT -> DELIVERED.
    /// @throws std::logic_error if current status is not SENT.
    void markAsDelivered();

    /// @brief Transitions SENDING or SENT -> UNDELIVERED.
    /// @throws std::logic_error if current status is DELIVERED or UNDELIVERED.
    void markAsUndelivered();

    /// @brief Transitions DELIVERED -> READ.
    /// @throws std::logic_error if current status is DELIVERED or UNDELIVERED.
    void markAsRead();

    // --- Content & Attachments ---
    /// @brief Edits the message content.
    /// @param newContent The new content.
    /// @throws std::logic_error if the message is not BUFFERED.
    void editContent(const MessageContent& newContent);

    /// @brief Adds an attachment ID to the message.
    /// @param attachmentId The ID of the attachment.
    /// @throws std::invalid_argument if the attachment is already present.
    void addAttachment(const AttachmentId& attachmentId);

    /// @brief Removes an attachment ID from the message.
    /// @param attachmentId The ID of the attachment.
    /// @throws std::invalid_argument if the attachment is not present.
    void removeAttachment(const AttachmentId& attachmentId);

    /// @brief Checks if the message has at least one attachment.
    bool hasAttachments() const noexcept
    { return !m_attachments.empty(); }

    // --- Validation ---
    /// @brief Checks if the message satisfies the "content OR attachment" invariant.
    /// @return true if content is not empty, or if there is at least one attachment.
    bool isValid() const noexcept;
};

} // namespace domain