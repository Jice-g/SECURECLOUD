#include "ui/LoginController.h"

#include <string>
#include <utility>

#include <QThread>

#include "application/loginusecase/AuthError.h"

namespace ui {

LoginController::LoginController(application::LoginUseCase& useCase, QObject* parent)
    : QObject(parent)
    , m_useCase(useCase)
{
}

LoginController::~LoginController()
{
    // A QThread must never be destroyed while running (Qt aborts).
    // The worst case is the network timeout (20 s, ADR UC1-14).
    if (m_thread != nullptr) {
        m_thread->wait();
    }
}

void LoginController::submit(const QString& login, const QString& password)
{
    if (m_thread != nullptr) {  // if a thread is running do not launch another one !
        return;   // one login at a time (the window also disables its button)
    }

    // result is a shared_ptr created in place in the heap :
    std::shared_ptr<WorkerResult> result = std::make_shared<WorkerResult>();

    application::LoginUseCase* const useCase = &m_useCase;

    std::string strLogin    = login.toStdString();
    std::string strPassword = password.toStdString();

    // The lambda owns COPIES of everything it touches: the worker never reads UI memory.
    // toStdString() gives UTF-8, which is exactly what the 256-byte rule measures (UC1-11).
    m_thread = QThread::create(
        [result, useCase, strLogin, strPassword](){
                try {
                    result->outcome = useCase->execute(strLogin, strPassword);
                    } catch (...) {
                        // L'exception ne doit pas franchir la frontière de thread.
                        // 'outcome' reste vide ; onWorkerFinished() signalera l'échec.
                    };
        });

    m_thread->setParent(this);
    m_result = std::move(result);  // moves the shared_ptr owner and not the contents
    connect(m_thread, &QThread::finished, this, &LoginController::onWorkerFinished);
    m_thread->start();
}

void LoginController::onWorkerFinished()
{
    // Back in the UI thread. Release our handles BEFORE emitting,
    // so a slot reacting to the signal may call submit() again.
    QThread* const thread = std::exchange(m_thread, nullptr);

    const std::shared_ptr<WorkerResult> result = std::move(m_result);
    if (thread != nullptr) {
        thread->deleteLater();
    }

    if (!result || !result->outcome.has_value()) {
        emit loginFailed(static_cast<int>(application::AuthError::INVALID_RESPONSE), 0);
        return;
    }

    const application::LoginOutcome& outcome = *result->outcome;
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
