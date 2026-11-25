/**
 * @file LoginController.h
 * @brief UI-side glue between the login window and LoginUseCase.
 */
#ifndef SC_UI_LOGINCONTROLLER_H
#define SC_UI_LOGINCONTROLLER_H

#include <memory>
#include <optional>

#include <QObject>
#include <QString>

#include "application/loginusecase/LoginOutcome.h"
#include "application/loginusecase/LoginProcessDTO.h"
#include "application/loginusecase/LoginUseCase.h"

class QThread;

namespace ui {

/**
 * @brief Runs LoginUseCase::execute() in a worker thread, so the UI never freezes.
 *
 * Threading story (UC1-04):
 *  - submit() is called in the UI thread; it COPIES the strings, then starts a QThread.
 *  - the worker runs the (blocking) use case and stores the outcome in a small shared object.
 *  - when the worker ends, QThread::finished is delivered to onWorkerFinished()
 *    IN THE UI THREAD (queued connection), which emits exactly one of the two signals.
 *  - no exception ever leaves the worker thread: it is caught and reported as INVALID_RESPONSE.
 *
 * Only QtCore + QtWidgets are needed (no QtConcurrent), matching the CMake configuration.
 * The signals carry plain ints/QStrings: no token, no Application type crosses to the widgets.
 */
class LoginController : public QObject
{
    Q_OBJECT

public:
    /**
     * @param useCase Use case to run. Must outlive the controller (guaranteed by the
     *                declaration order in main()). Not owned.
     * @param parent  Optional QObject parent.
     */
    explicit LoginController(application::LoginUseCase& useCase, QObject* parent = nullptr);

    /** Waits for a running worker (at most the network timeout) before destruction. */
    ~LoginController() override;

    /**
     * @brief Starts a login attempt. Ignored if one is already running.
     * @param login    Raw login as typed (the use case trims/normalizes it).
     * @param password Password as typed. Never modified, never logged.
     */
    void submit(const QString& login, const QString& password);

    /** @return true while a worker thread is running. */
    bool isBusy() const noexcept
    { return m_thread != nullptr; }

signals:
    /** @param role domain::UserRole cast to int. */
    void loginSucceeded(const QString& login, int role);

    /**
     * @param authError         application::AuthError cast to int.
     * @param retryAfterSeconds Delay for RATE_LIMITED, otherwise 0.
     */
    void loginFailed(int authError, int retryAfterSeconds);

private slots:
    void onWorkerFinished();

private:
    /** Written by the worker, read by the UI thread after QThread::finished. */
    struct WorkerResult
    {
        std::optional<application::LoginOutcome> outcome;   // empty = something threw
    };

    application::LoginUseCase& m_useCase;
    QThread* m_thread = nullptr;                // owned through QObject parentage
    std::shared_ptr<WorkerResult> m_result;
};


} // namespace ui

#endif // SC_UI_LOGINCONTROLLER_H
