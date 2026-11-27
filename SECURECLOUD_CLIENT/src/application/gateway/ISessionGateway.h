#pragma once

#include "application/loginusecase/AuthResult.h"
#include "application/loginusecase/LoginProcessDTO.h"

/// @file ISessionGateway.h

namespace application {

/// @brief PORT: the way the Application asks a server outside to authenticate a user.
/// @details Defined here (Application), implemented in Infrastructure (HttpsSessionGateway)
///          or in tests (FakeSessionGateway). The Application never knows about HTTP, JSON or Qt.
///          refresh() and revoke() will be added next to login() later.
///
class ISessionGateway {

public:
    virtual ~ISessionGateway() = default;

    /// @brief try to login.
    /// @param request validated login + raw password.
    /// @return a session or an error. Expected failures (wrong password, server down) are returned, not thrown.
    ///
    [[nodiscard]]
    virtual AuthResult login(const LoginRequestDTO& request) = 0;
};

} // namespace application
