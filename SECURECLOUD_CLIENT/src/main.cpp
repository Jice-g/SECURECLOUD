/**
 * @file main.cpp
 * @brief Composition root of the Client: the ONLY place that knows every layer.
 *
 * Flow (UC-01 login, UC-09 local sign-out):
 *
 *   UserLoginWindow --accepted--> ClientChatWindow
 *          ^                            |
 *          +------ signOutRequested ----+   (the session is closed first)
 *
 * Objects are created in dependency order and destroyed in the reverse order,
 * which guarantees that every reference below outlives its users.
 */
#include <QApplication>
#include <QCoreApplication>

#include "application/loginusecase/LoginUseCase.h"
#include "application/loginusecase/SessionManager.h"
#include "ui/ClientChatWindow.h"
#include "ui/LoginController.h"
#include "ui/Theme.h"
#include "ui/UserLoginWindow.h"


/* Environnement de test ou en production */
#if defined(SC_DEMO_GATEWAY)
#include "infrastructure/DemoSessionGateway.h"
#else
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QSslSocket>
#include <QUrl>
#include "infrastructure/GatewayConfig.h"
#include "infrastructure/HttpsSessionGateway.h"
#endif

namespace {

/** Brings a window to the front (needed when it is shown again after a hide). */
template <typename Window>
void present(Window& window)
{
    window.show();
    window.raise();
    window.activateWindow();
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("SecureCloud"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    ui::applyTheme(app);



    // ---- Infrastructure: the adapter behind the ISessionGateway port ----------------------

#if defined(SC_DEMO_GATEWAY)
    infrastructure::DemoSessionGateway gateway;          // scripted, no network (DEMO ONLY)

#else
    // Settings from the environment (ADR UC1-19). SC_DEV_CA is EMPTY by default: only the system
    // certificate authorities are trusted. In development, set SC_DEV_CA to the full path of dev-ca.pem.
    const auto env = QProcessEnvironment::systemEnvironment();
    infrastructure::GatewayConfig config;
    config.loginUrl   = QUrl(env.value(QStringLiteral("SC_GATEWAY_URL"), QStringLiteral("https://localhost:8443/auth/login")));
    config.caCertPath = env.value(QStringLiteral("SC_DEV_CA"), QString());

    // Refuse to start if TLS is not available or the settings are wrong (never fall back to plain http).
    if (!QSslSocket::supportsSsl()) {
        QMessageBox::critical(nullptr, QStringLiteral("SecureCloud"),
            QStringLiteral("Connexion sécurisée (TLS) indisponible sur ce poste : l'application ne peut pas démarrer."));
        return 1;
    }
    const QString configError = config.validate();
    if (!configError.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("SecureCloud"),
            QStringLiteral("Configuration invalide : ") + configError);
        return 1;
    }
    infrastructure::HttpsSessionGateway gateway(config);
#endif

    // ---- Application ----------------------------------------------------------------------

    application::SessionManager session;                       // empty = signed out

    application::LoginUseCase loginUseCase(gateway, session);  // sees only the PORT



    // ---- UI -------------------------------------------------------------------------------

// Login Controller is a thread
    ui::LoginController controller(loginUseCase);  // No thread: the use case and the gateway are asynchronous (signals),
                                                   // so the UI never freezes.

    ui::UserLoginWindow loginWindow(controller);

    ui::ClientChatWindow chatWindow;    


    // Login succeeded: show the main window.
    QObject::connect(&loginWindow, &QDialog::accepted, &chatWindow,
                    [&]() {
                         chatWindow.openSession(loginWindow.signedInLogin(), loginWindow.signedInRole());

                    #if defined(SC_DEMO_GATEWAY)
                            chatWindow.logInfo(QStringLiteral("DEMO backend: no network, no real authentication."));
                    #endif
                            present(chatWindow);
                        }
                     );

    // Login window closed without signing in (Esc / window button): quit if nothing else is open.
    QObject::connect(&loginWindow, &QDialog::rejected, &chatWindow,
                     [&]() {
                            if (!chatWindow.isVisible()) {
                                QCoreApplication::quit();
                            }
                        }
                     );



    // UC-09, local sign-out: forget the session, go back to the login screen.
    // (A dedicated LogoutUseCase will replace this direct call when logout gets server-side revocation.)
    QObject::connect(&chatWindow, &ui::ClientChatWindow::signOutRequested, &loginWindow, [&]() {
        session.close();
        loginWindow.prepareForNewSession();
        present(loginWindow);    // show the login first, so the app never has zero visible windows
        chatWindow.hide();
    });

    present(loginWindow);
    return app.exec();
}
