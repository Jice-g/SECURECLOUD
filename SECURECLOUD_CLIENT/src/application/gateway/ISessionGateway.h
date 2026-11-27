#pragma once

#include <QObject>

#include "application/loginusecase/AuthResult.h"
#include "application/loginusecase/LoginProcessDTO.h"

/// @file ISessionGateway.h

namespace application {

/// @brief PORT: the way the Application asks a server outside to authenticate a user.
/// @details Defined here (Application), implemented in Infrastructure (HttpsSessionGateway),
///          in the demo (DemoSessionGateway) or in tests (FakeSessionGateway).
///          ASYNCHRONOUS (ADR UC1-16): login() returns immediately, the answer comes through
///          the signal loginFinished(). Everything runs in the UI thread: no worker thread.
///          The Application never knows about HTTP or JSON. It may use Qt Core (ADR UC1-17).
///          refresh() and revoke() will be added next to login() later.
///
class ISessionGateway : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;
    ~ISessionGateway() override = default;

    /// @brief Starts a login and returns immediatly. No return : the result comes through loginFinished() connection.
    /// @param request validated login + raw password.
    /// @details An implementation may emit loginFinished() before login() returns
    ///          (see the fake in the tests): the caller must be ready for that (LoginUseCase::execute()).
    ///
    virtual void login(const LoginRequestDTO& request) = 0;

signals:
    /// @brief Exactly one emission per login() call. A success carries a session, a failure an AuthError.
    /// @details Same thread, direct connection: NEVER connect it with Qt::QueuedConnection
    ///          (AuthResult is not registered as a Qt metatype).
    ///
    void loginFinished(const application::AuthResult& result);
};

} // namespace application
