#include <exception>
#include <optional>
#include <stdexcept>

#include "domain/model/UserLogin.h"
#include "application/loginusecase/LoginUseCase.h"

namespace application {

LoginUseCase::LoginUseCase(ISessionGateway& gateway, SessionManager& session, QObject* parent)
    : QObject(parent), m_gateway(gateway), m_session(session)
{
    // Same thread, so this is a direct call (never a queued connection, see ISessionGateway).
    connect(&m_gateway, &ISessionGateway::loginFinished, this, &LoginUseCase::onGatewayFinished);
}

bool LoginUseCase::isBusy() const noexcept
{
    return m_busy;
}

void LoginUseCase::execute(const std::string& strLogin, const std::string& password)
{
    // 0. A login is already running: this call is ignored (1 answer per accepted call).
    if (m_busy) {
        return;
    }

    // 1. Local validation before any network call. The Login Business rules applied here.
    //    std::string::size() counts bytes (UTF-8), which is what the gateway contract counts.
    if (password.empty() || password.size() > MAX_PASSWORD_BYTES) {
        emit finished(LoginOutcome::failure(AuthError::INVALID_INPUT));
        return;
    }

    // Building a UserLogin IS the login validation (it trims, lowercases, then throws if invalid).
    std::optional<domain::UserLogin> login;
    try {
        login.emplace(strLogin);
    } catch (const std::invalid_argument&) {
        emit finished(LoginOutcome::failure(AuthError::INVALID_INPUT));
        return;
    }

    // 2. Ask the port. m_busy is set BEFORE the call: a fake may answer before login() returns,
    //    and onGatewayFinished() ignores any answer received while not busy.
    m_busy = true;
    try {
        m_gateway.login(LoginRequestDTO{*login, password});
    } catch (const std::exception&) {
        // A technical failure of the adapter: the use case is free again and tells the UI.
        m_busy = false;
        emit finished(LoginOutcome::failure(AuthError::INVALID_RESPONSE));
    }
}

void LoginUseCase::onGatewayFinished(const AuthResult& result)
{
    if (!m_busy) {
        return;  // unexpected answer, nobody is waiting for it
    }
    m_busy = false;  // BEFORE emitting: a slot connected to finished() may call execute() again

    if (!result.ok()) {
        emit finished(LoginOutcome::failure(result.error(), result.retryAfterSeconds()));
        return;
    }

    // Success: keep the session (RAM only), tell the UI who is connected, never the tokens.
    m_session.open(result.session());
    const domain::User& user = result.session().user();
    emit finished(LoginOutcome::success(LoginResultDTO{user.login().value(), user.role()}));
}

} // namespace application
