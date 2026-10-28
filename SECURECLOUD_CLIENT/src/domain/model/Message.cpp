#include "domain/model/Message.h"

/// @file   Message.cpp
/// @brief  Implementation of the Message entity's business logic.

namespace domain {

void Message::markAsBuffered() {
    if (m_status != MessageStatus::DRAFTED || m_status != MessageStatus::UNDELIVERED ) {
        throw std::logic_error("Only DRAFTED or UNDELIVERED messages can transition to BUFFERED");
    }
    m_status = MessageStatus::SENDING;
}

void Message::markAsSending() {
    if (m_status != MessageStatus::BUFFERED) {
        throw std::logic_error("Only BUFFERED messages can transition to SENDING");
    }
    m_status = MessageStatus::SENDING;
}

void Message::markAsSent() {
    if (m_status != MessageStatus::SENDING) {
        throw std::logic_error("Only SENDING messages can transition to SENT");
    }
    m_sentAt = TimeStamp::now();
    m_status = MessageStatus::SENT;
}

void Message::markAsDelivered() {
    if (m_status != MessageStatus::SENT) {
        throw std::logic_error("Only SENT messages can transition to DELIVERED");
    }
    m_status = MessageStatus::DELIVERED;
}

void Message::markAsUndelivered() {
    if (m_status != MessageStatus::SENDING && m_status != MessageStatus::SENT) {
        throw std::logic_error("Only SENDING or SENT messages can transition to UNDELIVERED");
    }
    m_status = MessageStatus::UNDELIVERED;
}

void Message::markAsRead(){
    if (m_status != MessageStatus::DELIVERED) {
        throw std::logic_error("Only DELIVERED messages can transition to READ");
    }
    m_status = MessageStatus::READ;
}

void Message::editContent(const MessageContent& newContent) {
    if (m_status != MessageStatus::BUFFERED) {
        throw std::logic_error("Only BUFFERED messages can be edited");
    }
    m_content = newContent;
}

void Message::addAttachment(const AttachmentId& attachmentId) {
    auto it = std::find(m_attachments.begin(), m_attachments.end(), attachmentId);
    if (it != m_attachments.end()) {
        throw std::invalid_argument("Attachment is already linked to this message");
    }
    m_attachments.push_back(attachmentId);
}

void Message::removeAttachment(const AttachmentId& attachmentId) {
    auto it = std::find(m_attachments.begin(), m_attachments.end(), attachmentId);
    if (it == m_attachments.end()) {
        throw std::invalid_argument("Attachment not found in this message");
    }
    m_attachments.erase(it);
}

bool Message::isValid() const noexcept {
    // Invariant business rule : Content OR Attachment
    return !m_content.isEmpty() || !m_attachments.empty();
}

} // namespace domain

