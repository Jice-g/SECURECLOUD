#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm> // For std::find

#include "domain/model/UserId.h"
#include "domain/model/Enums.h"         // ConversationType
#include "domain/model/TimeStamp.h"
#include "domain/model/ConversationId.h"

/// @file   Conversation.h
/// @brief  Polymorphic Entity representing a conversation (direct or group).
///
/// The base class Conversation is abstract.
/// With common identity (id, createdAt) and common behavior (hasParticipant).
///


/*
 * The derived classes are :
 *
 * - DirectConversation : exactly 2 different participants, no title.
 * - GroupConversation  : 1 and more participants, requires a title.
 *
 * This file is header-only.
 */


namespace domain {

/* **************************************************************************
 *          Base Class: Conversation (Abstract)
 * **************************************************************************/


/// @brief Abstract base class for all conversations.
///
/// Cannot be instantiated directly. Use DirectConversation or GroupConversation.
///
class Conversation {

protected:
    // 'protected' : only derived classes access it.
    ConversationId  m_id;
    TimeStamp       m_createdAt;

public:
    /// @brief Constructs the common base of a Conversation.
    /// @param id The unique identifier.
    /// @param createdAt The creation timestamp.
    Conversation(const ConversationId& id, const TimeStamp& createdAt)
        : m_id(id), m_createdAt(createdAt) {}

    /// @brief Virtual destructor.
    /// @note Without 'virtual', deleting a derived object
    ///       would cause undefined behavior (memory leak).
    virtual ~Conversation() = default;

    // Prevent copying to avoid object (as Conversation) slicing (good practice for entities)
    Conversation(const Conversation&) = delete;
    Conversation& operator=(const Conversation&) = delete;

    /// @brief Returns the unique identifier of the conversation.
    const ConversationId& id() const noexcept
    { return m_id; }

    /// @brief Returns the creation timestamp.
    const TimeStamp& createdAt() const noexcept
    { return m_createdAt; }

    /// @brief Pure virtual method: returns the type of the conversation.
    /// @note Every derived class must implement it with enum DIRECT or GROUP
    virtual ConversationType type() const noexcept = 0;

    /// @brief Pure virtual method: checks if a specific user is in this conversation.
    /// @param userId The user to check.
    /// @return true if the user is a participant.
    virtual bool isParticipating(const UserId& userId) const noexcept = 0;
};



/* *******************************************************************
 * Derived Class: DirectConversation
 * *******************************************************************/

/// @brief Entity representing a 1-to-1 conversation.
///
/// Invariants:
///   - Exactly two participants.
///   - No title (the UI displays the other participant's name).
///
class DirectConversation final : public Conversation {
private:
    UserId m_participant1;
    UserId m_participant2;

public:
    /// @brief Constructs a new DirectConversation.
    /// @param id The unique identifier.
    /// @param p1 The first participant.
    /// @param p2 The second participant.
    /// @param createdAt The creation timestamp.
    ///
    DirectConversation(const ConversationId& id, const UserId& p1, const UserId& p2, const TimeStamp& createdAt)
        : Conversation(id, createdAt), m_participant1(p1), m_participant2(p2)
    { }

    /// @brief Returns the type (DIRECT).
    ConversationType type() const noexcept override
    { return ConversationType::DIRECT; }

    /// @brief Checks if a user is one of the two participants.
    bool isParticipating(const UserId& userId) const noexcept override {
        return m_participant1 == userId || m_participant2 == userId;
    }

    /// @brief Returns the first participant : The first transmitter
    const UserId& participant1() const noexcept
    { return m_participant1; }

    /// @brief Returns the second participant.
    const UserId& participant2() const noexcept
    { return m_participant2; }

    /// @brief There cannot be 2 different direct conversations with the same two participants.
    /// A conversation is UserId_1 + UserId_2 conversing with each other.
    ///
    bool checkParticipants(const UserId& user1, const UserId& user2) const noexcept
    {
       return
        ( this->participant1() == user1  &&  this->participant2() == user2 )
        || ( this->participant1() == user2  &&  this->participant2() == user1 );
    }

};



// =============================================================================
// Derived Class: GroupConversation
// =============================================================================

/// @brief Entity representing a group conversation.
///
/// Invariants:
///   - Requires a non-empty title.
///   - Can have any number of participants.
///   - Participants can be added or removed.
class GroupConversation final : public Conversation {
private:
    std::string m_title;
    std::vector<UserId> m_participants;

public:
    /// @brief Constructs a new GroupConversation.
    /// @param id The unique identifier.
    /// @param title The name of the group. Must not be empty.
    /// @param createdAt The creation timestamp.
    /// @throws std::invalid_argument if title is empty.
    GroupConversation(const ConversationId& id,
                      const std::string& title,
                      const TimeStamp& createdAt)
        : Conversation(id, createdAt), m_title(title)
    {
        if (m_title.empty()) {
            throw std::invalid_argument("Group conversation must have a title");
        }
    }

    /// @brief Returns the type (GROUP).
    ConversationType type() const noexcept override { return ConversationType::GROUP; }

    /// @brief Checks if a user is in the group.
    bool isParticipating(const UserId& userId) const noexcept override {
        return std::find(m_participants.begin(), m_participants.end(), userId) != m_participants.end();
    }

    /// @brief Returns the title of the group.
    const std::string& title() const noexcept { return m_title; }

    /// @brief Returns the list of participants.
    const std::vector<UserId>& participants() const noexcept { return m_participants; }

    /// @brief Adds a participant to the group.
    /// @param userId The user to add.
    /// @throws std::invalid_argument if the user is already in the group.
    void addParticipant(const UserId& userId) {
        if (isParticipating(userId)) {
            throw std::invalid_argument("User is already a participant in this group");
        }
        m_participants.push_back(userId);
    }

    /// @brief Removes a participant from the group.
    /// @param userId The user to remove.
    /// @throws std::invalid_argument if the user is not in the group.
    void removeParticipant(const UserId& userId) {
        auto it = std::find(m_participants.begin(), m_participants.end(), userId);
        if (it == m_participants.end()) {
            throw std::invalid_argument("User is not a participant in this group");
        }
        m_participants.erase(it);
    }
};

} // namespace domain