#pragma once

#include <optional>

#include "domain/model/Enums.h"
#include "domain/model/UserId.h"
#include "domain/model/UserLogin.h"
#include "domain/model/TimeStamp.h"

namespace domain {

/// @file User.h
/// @brief Represents a user of SecureCloud Client.
/// @details The User class encapsulates the essential attributes and behaviors of a user in the SecureCloud system.
/// 
/// No .cpp file is required as this is a header-only class with simple getters and setters.
/// The class is marked as final to prevent inheritance, ensuring that the user entity remains consistent and secure.
/// 
class User final {

    private:

        UserId m_id;
        UserLogin m_login;
        UserRole m_role;
        OnlineStatus m_presence = OnlineStatus::OFFLINE;
        TimeStamp m_createdAt = TimeStamp::now();

    public:

        /// @brief Constructs a User with the specified attributes.
        /// @details The constructor initializes the user with a unique identifier, login, role, presence status, and creation timestamp.
        /// @param id The unique identifier for the user.
        /// @param login The login assigned to the user.
        /// @param role The role assigned to the user.
        /// @param presence The presence status of the user. Defaults to OFFLINE if not specified.
        ///  - createdAt The timestamp indicating when the user was created is automatically set to the current time.
        ///  - lastUserLogin The timestamp of the user's last login is optional. The user may not have logged in yet, in which case this value will be empty.
        ///
        User(const UserId& id, const UserLogin& login, UserRole role, TimeStamp createdAt, OnlineStatus presence = OnlineStatus::OFFLINE)
            : m_id(id), m_login(login), m_role(role), m_createdAt(createdAt),  m_presence(presence) { }


        // Accessors (getters) for the User attributes :

        [[nodiscard]]
        const UserId& Id() const noexcept
        { return m_id; }

        [[nodiscard]]
        const UserLogin& Login() const noexcept
        { return m_login; }

        [[nodiscard]]
        UserRole Role() const noexcept
        { return m_role; }

        [[nodiscard]]
        OnlineStatus Presence() const noexcept
        { return m_presence; }

        [[nodiscard]]
        bool IsOnline() const noexcept
        {
            if (m_presence == OnlineStatus::ONLINE){
                return true; }
            return false;
        }

        [[nodiscard]]
        bool IsOffline() const noexcept
        {
            if (m_presence == OnlineStatus::OFFLINE){
                return true; }
            return false;
        }

        [[nodiscard]]
        const TimeStamp& CreatedAt() const noexcept
        { return m_createdAt; }


       // Mutators (setters) for the User attributes :

        void Online()
        {   m_presence = OnlineStatus::ONLINE;
        }

        void Offline()
        {
            m_presence = OnlineStatus::OFFLINE;
         }

        void Away()
        {
            if (m_presence == OnlineStatus::ONLINE) // Allow setting to AWAY when the user is currently ONLINE
            m_presence = OnlineStatus::AWAY;
        }

        void setRole(UserRole newRole)
        { m_role = newRole; }

};

}