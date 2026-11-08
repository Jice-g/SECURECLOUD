#pragma once

#include <optional>
#include <vector>

#include "domain/model/User.h"
#include "domain/model/UserId.h"
#include "domain/model/UserLogin.h"

/// @file   IUserRepository.h
/// @brief  Domain contract: access to known Users (the local contact book).
///
/// The Client does not own the user directory (the server does). This
/// repository is the local copy used for contacts and name resolution.
/// States WHAT the domain needs, never HOW (SQLite, CSV...): that is the
/// business of Infrastructure.

namespace domain {

class IUserRepository {
	
public:

    virtual ~IUserRepository() = default;
/*
    /// @brief Inserts the user, or updates it if the id already exists (upsert).
    /// @throws RepositoryError on storage failure.
    virtual void save(const User& user) = 0;
maybe postponed in a V1 or V2 */

    /// @return the user, or std::nullopt if unknown.
    [[nodiscard]]
	virtual std::optional<User> findById(const UserId& id) const = 0;

    /// @return the user, or std::nullopt if unknown. Logins are normalized (UserLogin).
    [[nodiscard]]
	virtual std::optional<User> findByLogin(const UserLogin& login) const = 0;

    /// @return all known users (contact book), unspecified order.
	/// @note the UI will need it to display all users
	///          so the current user of the client can pick the ones he wants
    [[nodiscard]]
	virtual std::vector<User> findAll() const = 0;
};

} // namespace domain
