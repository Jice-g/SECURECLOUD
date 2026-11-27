#pragma once

#include <cstddef>
#include <string>

#include <QObject>

#include "application/gateway/ISessionGateway.h"
#include "application/loginusecase/LoginOutcome.h"
#include "application/loginusecase/SessionManager.h"
#include "application/loginusecase/LoginProcessDTO.h"

/// @file LoginUseCase.h

namespace application {

/// @brief Use case #1 (UC-01) : "log in". It validates the input, asks the port, opens the session.
/// @details ASYNCHRONOUS (ADR UC1-16): execute() returns at once, the outcome arrives through the signal finished().
///          Everything runs in the UI thread (no worker thread, no mutex needed here).
///          It knows the PORT only (ISessionGateway), never the HTTPS adapter:
///          that makes it testable with a fake, without network.
///          It holds references: the gateway and the session manager must outlive it.
///
class LoginUseCase final : public QObject {
    Q_OBJECT

public:
    /// Maximum password size, in BYTES of UTF-8 (the contract with the Gateway counts bytes, not characters).
    /// The reason is some special characters that may be coded on 2 bytes.
    ///
    static constexpr std::size_t MAX_PASSWORD_BYTES = 256;

    /// @param gateway  the port used to authenticate (& : not owned), instanciated in the main.
    /// @param session  where the session is stored on success (& : not owned), instanciated in the main.
    /// @param parent   optional Qt parent.
    /// @details The constructor connects gateway.loginFinished() to onGatewayFinished().
    ///
    LoginUseCase(ISessionGateway& gateway, SessionManager& session, QObject* parent = nullptr);

    /// @brief Starts the login scenario. Returns immediately.
    /// @param strLogin login as typed by the user. Spaces and case are normalized by UserLogin (domain).
    /// @param password as typed, [1..MAX_PASSWORD_BYTES] bytes.
    /// @details Ignored if a login is already running (isBusy()).
    ///          Bad input: finished(INVALID_INPUT) is emitted at once, the gateway is NOT called.
    ///          If the gateway throws a std::exception: finished(INVALID_RESPONSE), the use case is free again.
    ///          Exactly one finished() per accepted execute(). It may be emitted BEFORE execute() returns.
    ///
    void execute(const std::string& strLogin, const std::string& password);

    /// @brief true between an accepted execute() and its finished().
    [[nodiscard]]
    bool isBusy() const noexcept;

signals:
    /// @brief The outcome: a user (login + role, NEVER a token) or an AuthError.
    void finished(const application::LoginOutcome& outcome);

private slots:
    void onGatewayFinished(const application::AuthResult& result);

private:
    ISessionGateway& m_gateway;
    SessionManager&  m_session;
    bool             m_busy = false;
};

} // namespace application
