/**
 * @file LoginController.h
 * @brief UI-side glue between the login window and LoginUseCase.
 */
#ifndef SC_UI_LOGINCONTROLLER_H
#define SC_UI_LOGINCONTROLLER_H

#include <QObject>
#include <QString>

#include "application/loginusecase/LoginOutcome.h"
#include "application/loginusecase/LoginUseCase.h"

namespace ui {

/**
 * @brief Translates between the widgets (QString, int) and LoginUseCase (std::string, LoginOutcome).
 *
 * No thread (ADR UC1-16): LoginUseCase is asynchronous, the whole login runs in the UI thread
 * and the UI never freezes because nothing blocks. The use case answers through its signal
 * finished(); this controller turns that answer into exactly one of its two signals.
 *
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
     * The constructor connects LoginUseCase::finished() to onUseCaseFinished().
     */
    explicit LoginController(application::LoginUseCase& useCase, QObject* parent = nullptr);

    /**
     * @brief Starts a login attempt and returns at once. Ignored if one is already running.
     * @param login    Raw login as typed (the use case trims/normalizes it).
     * @param password Password as typed. Never modified, never logged.
     * @details One of the two signals below follows, possibly BEFORE submit() returns (bad input).
     */
    void submit(const QString& login, const QString& password);

    /** @return true while a login is running (asked to the use case). */
    bool isBusy() const noexcept
    { return m_useCase.isBusy(); }

signals:
    /** @param role domain::UserRole cast to int. */
    void loginSucceeded(const QString& login, int role);

    /**
     * @param authError         application::AuthError cast to int.
     * @param retryAfterSeconds Delay for RATE_LIMITED, otherwise 0.
     */
    void loginFailed(int authError, int retryAfterSeconds);

private slots:
    void onUseCaseFinished(const application::LoginOutcome& outcome);

private:
    application::LoginUseCase& m_useCase;
};

} // namespace ui

#endif // SC_UI_LOGINCONTROLLER_H
