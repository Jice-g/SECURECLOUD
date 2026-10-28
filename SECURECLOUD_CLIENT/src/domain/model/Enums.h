#pragma once


/*
 * All enumerations in this file are `enum class` types (not enum) = idiomatic C++.
 * They define the fundamental state values used by the Domain entities
 * and they are exposed to upper layers (DTOs, mappers...).
 * Header only. No behavior.
 * ---
 * The enums.h is exposed to every layer accessing Domain (that means all the classes of the SecureCloud_Client application).
 * Thus, we've put all the (#include) libraries used by the domain classes in this file (once for all),
 * so that the libraries can be used in any Domain class without additional includes in the files that include this header.
 * All enums are defined in this file, along with their documentation and other constants if needed.
 * ---
 */

 ///@file Enums.h
 ///@brief Domain enumerations
 /// 
namespace domain {


 ///@brief Role of a User in SecureCloud system.
 ///@details : Determines the permissions of the user.
 /// 
 enum class UserRole {
    ADMIN,      ///< Administrative privileges.
    COORD,       ///< Organization-level privileges (exemple : MSF coordinator).
    STD_USER    ///< Standard user.
 };

 ///@brief On Line status of a user.
 ///
 enum class OnlineStatus {
    ONLINE,     ///< User is connected and available.
    OFFLINE,    ///< User is disconnected.
	AWAY        ///< User is connected but temporarily unavailable / maybe implemented in the future V1
 }; 


 ///@brief Priority level of a message or an alert.
 ///@details : Used by the MessagingService to order delivery and by the UI to
 ///           highlight critical messages (ex :. medical emergencies).
 /// 
 enum class Priority {
    NORMAL,   ///< Default priority for normal messages.
    HIGH,       ///< Important message requiring attention.
    CRITICAL    ///< Emergency-level message (exemple : surgical emergency).
 };
 

 ///@brief Lifecycle status of a message.
 ///@details : State of the Message entity:
 /*
  *  DRAFT ──(Envoyer)──► BUFFERED ──► SENDING ──► SENT ──► DELIVERED ──► READ
  *                         ▲            │          │
  *                         │            ▼          ▼
  *                   retry └────── UNDELIVERED ◄───┘   (requeue())
  *
  * A message cannot skip states (for example, it cannot go from BUFFERED directly to DELIVERED).
  */
enum class MessageStatus {
    DRAFTED,       ///< Is presently being written and completed by the user
    BUFFERED,      ///< Stored locally, not yet transmitted.
    SENDING,       ///< Transmission to the API Gateway in progress.
    SENT,          ///< Acknowledged by the server, awaiting delivery.
    DELIVERED,     ///< Acknowledged by the recipient's device.
    UNDELIVERED,   ///< Transmission failed (network error, timeout).
    READ           ///< Message has been read by the recipient.
};


 /// @brief Type of a conversation.
 /// @details : Determines the rules concerning the Conversation entity and inherited classes:
 /// - DIRECT : only 2 participants (1-to-1 conversation), no title required.
 /// - GROUP  : 2 or more participants, a title is required.
enum class ConversationType {
    DIRECT,   ///< One-to-one conversation 'DirectConversation'
    GROUP     ///< One-to-many conversation 'GroupConversation'
};

} // namespace domain


