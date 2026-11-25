#pragma once

#include <string>

#include "domain/model/UserLogin.h"

/// @file LoginProcessDTO.h

namespace application {

/// @brief What the port ISessionGateway needs to attempt a login : login and password.
/// @details Building the UserLogin is a validation.
///          The password is a raw UTF-8 string, NEVER trimmed or normalized.
///          Size rule (1..256 bytes) is checked by LoginUseCase.
///
struct LoginRequestDTO {
    domain::UserLogin login;  ///< Already normalized (trimmed, lowercase) by the domain.
    std::string       password;
};

/* Ther UI sends a login and a password (2 strings) to application layer
 * The application layer send a UserLogin and a Passsword in a LoginRequestDTO to the Gateway
 * through infrastructure layer
 * The Gateway replies with a valid login and a userRole
 * The result is sent to the UI
 */


/// @brief What the UI may know about a successful login. NEVER a token:
///        secrets do not leave the Application layer.
///
struct LoginResultDTO
{
    std::string     login;  ///< Normalized login of the connected user.
    domain::UserRole role;  ///< Display hint only; the server checks permissions on every request.
};

} // namespace application
