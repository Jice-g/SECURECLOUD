#pragma once

#include <cstddef>
#include <string>

#include "application/gateway/ISessionGateway.h"
#include "application/loginusecase/LoginOutcome.h"
#include "application/loginusecase/SessionManager.h"
#include "application/loginusecase/LoginProcessDTO.h"

/// @file LoginUseCase.h

namespace application {

/// @brief Use case #1 (UC-01) : "log in". It validates the input, asks the port, opens the session.
/// @details Synchronous. The UI runs it in a worker thread (LoginController).
///          It knows the PORT only (ISessionGateway), never the HTTPS adapter:
///          that makes it testable with a fake (the DemoSessionGateway), without network and without Qt.
///          It holds references: the gateway and the session manager must exist independantly.
///
class LoginUseCase final {

public:
    /// Maximum password size, in BYTES of UTF-8 (the contract with the Gateway counts bytes, not characters).
    /// The reason is some special caracters that may be coded on 2 bytes
    ///
    static constexpr std::size_t MAX_PASSWORD_BYTES = 256;

    /// @param gateway  the port used to authenticate (& : not owned).
    /// @param session where the session is stored on success. (& : not owned).
    /// @details The real gateway is defined and reached in the infrastruture layer.
    ///
    LoginUseCase(ISessionGateway& gateway, SessionManager& session);

    /// @brief Runs the login scenario. Waits for the result.
    /// @param strLogin login as it is typed by the client current user. Spaces and case are normalized by UserLogin(domain).
    /// @param password as it was typed [1..MAX_PASSWORD_BYTES] bytes.
    /// @return a LoginOutcome: a user or an error. user : login + role (never a token).
    ///         INVALID_INPUT is returned before any call to the gateway.
    ///         A failure never opens a session and never closes an already open one.
    /// @throws whatever the gateway throws on a technical failure: nothing is caught here;
    ///         the UI controller catches it and displaies the error message.
    ///
    [[nodiscard]]
    LoginOutcome execute(const std::string& strLogin, const std::string& password);

private:
    ISessionGateway& m_gateway;
    SessionManager&  m_session;
};

} // namespace application
