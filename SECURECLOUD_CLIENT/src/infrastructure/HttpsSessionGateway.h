#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QNetworkAccessManager>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QString>
#include <QTimer>

#include "application/gateway/ISessionGateway.h"
#include "infrastructure/GatewayConfig.h"

class QNetworkReply;

/// @file HttpsSessionGateway.h

namespace infrastructure {

/// @brief The REAL adapter of the port ISessionGateway: logs in through HTTPS (POST /auth/login).
/// @details ASYNCHRONOUS and single-threaded (ADR UC1-16): login() sends the request and returns at once;
///          the answer arrives later through the Qt event loop and is delivered by loginFinished().
///          Exactly ONE loginFinished() per login() call.
///          Security (check-list of the ADR): TLS 1.2 or later, the server certificate AND its host name are
///          verified, the dev CA is ADDED to the system CAs, ignoreSslErrors() is never called,
///          redirections are never followed, no Authorization header.
///          The decision is taken from the HTTP STATUS (not from QNetworkReply::error()): no status at all
///          means SERVER_UNREACHABLE.
///          Logs: request id, status, error name, duration. NEVER the body, the password or a token.
///
class HttpsSessionGateway final : public application::ISessionGateway {
    Q_OBJECT

public:
    /// @param config the settings (validated here; an invalid config makes every login() fail).
    ///        The dev CA file is read once, here.
    explicit HttpsSessionGateway(GatewayConfig config, QObject* parent = nullptr);

    /// @brief Aborts a running request, silently (no signal is emitted).
    ~HttpsSessionGateway() override;

    /// @brief Sends the login request and returns immediately. The result comes through loginFinished().
    /// @details An invalid config, or a request already running, gives SERVER_UNREACHABLE (emitted at once).
    void login(const application::LoginRequestDTO& loginRequest) override;

private slots:
    void onReplyFinished();   ///< the network answered (or failed, or was aborted)
    void onDeadline();        ///< the total time is over: abort the request

private:
    /// Logs (id, status, result, duration) and emits loginFinished(). The ONLY place that emits.
    void finish(const application::AuthResult& result, int httpStatus);

    GatewayConfig            m_config;
    QString                  m_configError;     ///< "" if the config is valid
    QSslConfiguration        m_sslConfig;       ///< TLS settings, built once
    QNetworkAccessManager    m_manager;         ///< a MEMBER: it lives as long as the gateway
    QNetworkReply*           m_reply = nullptr; ///< the running request (not owned by us: the manager owns it)
    QTimer                   m_deadline;        ///< single-shot: the TOTAL time allowed
    bool                     m_timedOut = false;
    QElapsedTimer            m_elapsed;
    QString                  m_requestId;
};

} // namespace infrastructure
