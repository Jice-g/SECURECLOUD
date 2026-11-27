#include "ui/LoginController.h"

#include <string>

#include "application/loginusecase/AuthError.h"

namespace ui {

LoginController::LoginController(application::LoginUseCase& useCase, QObject* parent)
    : QObject(parent)
    , m_useCase(useCase)
{
    // Same thread: a direct connection, the slot runs inside the emission of finished().
    connect(&m_useCase, &application::LoginUseCase::finished, this, &LoginController::onUseCaseFinished);
}

void LoginController::submit(const QString& login, const QString& password)
{
    // If a login is running the use case ignores this call (the window also disables its button).
    // toStdString() gives UTF-8, which is exactly what the 256-byte rule measures (UC1-11).
    m_useCase.execute(login.toStdString(), password.toStdString());
}

void LoginController::onUseCaseFinished(const application::LoginOutcome& outcome)
{
    if (outcome.user.has_value()) {
        emit loginSucceeded(QString::fromStdString(outcome.user->login), static_cast<int>(outcome.user->role));
    } else if (outcome.error.has_value()) {
        emit loginFailed(static_cast<int>(*outcome.error), outcome.retryAfterSeconds);
    } else {
        // Neither success nor error: the use case broke its own contract.
        emit loginFailed(static_cast<int>(application::AuthError::INVALID_RESPONSE), 0);
    }
}

} // namespace ui
