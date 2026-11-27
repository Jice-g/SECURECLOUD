#include <optional>
#include <stdexcept>

#include "domain/model/UserLogin.h"
#include "application/loginusecase/LoginUseCase.h"

namespace application {

LoginUseCase::LoginUseCase(ISessionGateway& gateway, SessionManager& session)
    : m_gateway(gateway), m_session(session)
{ }

LoginOutcome LoginUseCase::execute(const std::string& strLogin, const std::string& password)
{
/***
 *  1. Local validation before any network call :
 *  - std::string::size() counts bytes (UTF-8). No more.
 *  - try to build a UserLogin (checked by the value object of the domain business rules
 * No more.
 ***/
    if (password.empty() || password.size() > MAX_PASSWORD_BYTES) {
        return LoginOutcome::failure(AuthError::INVALID_INPUT);
    }

    // Building a UserLogin IS the login validation (it trims, lowercases, then throws if invalid).
    std::optional<domain::UserLogin> login;
    try {
        login.emplace(strLogin); // build the UserLogin directly here (emplace)
    } catch (const std::invalid_argument&) {
        return LoginOutcome::failure(AuthError::INVALID_INPUT);
    }
// Now the incomes are correct regarding business rules

/***
 *   2. Ask the port.
 *    - Expected failures come back as values;
 *    - technical failures propagate as exceptions.
 ***/
    // Send the structure LoginRequestDTO to the Gateway
    // don't forget *login (the value, not the object)
    const AuthResult result = m_gateway.login(LoginRequestDTO{*login, password});
    if (!result.ok()) {
        return LoginOutcome::failure(result.error(), result.retryAfterSeconds());
    }

    // 3. Success: keep the session (RAM only) and tell the UI who is connected, never the tokens.
    m_session.open(result.session());
    const domain::User& user = result.session().user();

    return LoginOutcome::success(LoginResultDTO{user.login().value(), user.role()});
}

} // namespace application
