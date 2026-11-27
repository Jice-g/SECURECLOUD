#include "infrastructure/HttpsSessionGateway.h"

#include <utility>

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QUuid>

#include "domain/model/TimeStamp.h"
#include "infrastructure/AuthJsonMapper.h"

namespace infrastructure {

using application::AuthError;
using application::AuthResult;

namespace {

/// Retry-After: ONLY the form "number of seconds" is read (not the HTTP date). Anything else gives 0 ("unknown").
int parseRetryAfter(const QByteArray& header)
{
    bool ok = false;
    const int seconds = QString::fromLatin1(header.trimmed()).toInt(&ok);
    return (ok && seconds > 0) ? seconds : 0;
}

} // namespace

HttpsSessionGateway::HttpsSessionGateway(GatewayConfig config, QObject* parent)
    : application::ISessionGateway(parent), m_config(std::move(config))
{
    m_configError = m_config.validate();

    // TLS settings, built once. Start from the defaults (they hold the SYSTEM certificate authorities).
    m_sslConfig = QSslConfiguration::defaultConfiguration();
    m_sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    if (m_configError.isEmpty()) {
        m_sslConfig.addCaCertificates(m_config.loadCaCertificates());   // ADDED, the system CAs stay
    }
    // The host name check is left to its default (on): we never touch it.

    m_deadline.setSingleShot(true);
    connect(&m_deadline, &QTimer::timeout, this, &HttpsSessionGateway::onDeadline);
}

HttpsSessionGateway::~HttpsSessionGateway()
{
    if (m_reply != nullptr) {
        m_reply->disconnect(this);   // so that abort() does NOT call onReplyFinished() (silent)
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void HttpsSessionGateway::login(const application::LoginRequestDTO& loginRequest)
{
    m_requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_elapsed.start();

    // Defence in depth (main.cpp validates first): bad config or a request already running.
    if (!m_configError.isEmpty() || m_reply != nullptr) {
        finish(AuthResult::failure(AuthError::SERVER_UNREACHABLE), 0);
        return;
    }

    QNetworkRequest httpRequest(m_config.loginUrl);
    httpRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json; charset=utf-8"));
    httpRequest.setRawHeader("Accept", "application/json");
    httpRequest.setRawHeader("X-Request-Id", m_requestId.toUtf8());
    httpRequest.setHeader(QNetworkRequest::UserAgentHeader,
                          QStringLiteral("SecureCloud-Client/") + QCoreApplication::applicationVersion());
    // No Authorization header: the login route is public.

    httpRequest.setSslConfiguration(m_sslConfig);
    httpRequest.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);   // HTTP/1.1 is enough (contract Q8)
    httpRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    httpRequest.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    // Qt 6 follows some redirections by default: the password could be sent elsewhere. We follow NONE.
    httpRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

    m_timedOut = false;
    m_reply = m_manager.post(httpRequest, AuthJsonMapper::buildLoginBody(loginRequest));
    connect(m_reply, &QNetworkReply::finished, this, &HttpsSessionGateway::onReplyFinished);

    m_deadline.start(m_config.totalTimeoutMs);   // TOTAL time (connection + answer), no retry
    // returns immediately: the answer comes through onReplyFinished()
}

void HttpsSessionGateway::onDeadline()
{
    if (m_reply == nullptr) {
        return;
    }
    m_timedOut = true;
    m_reply->abort();   // makes the reply emit finished(): onReplyFinished() will answer SERVER_UNREACHABLE
}

void HttpsSessionGateway::onReplyFinished()
{
    m_deadline.stop();

    // Take the reply out of the member FIRST, so that the object is free again whatever happens below.
    QNetworkReply* reply = m_reply;
    m_reply = nullptr;
    if (reply == nullptr) {
        return;
    }
    reply->deleteLater();

    // No HTTP status at all (DNS, TCP, TLS refused, connection cut) or time over: SERVER_UNREACHABLE.
    // The status is the criterion, NOT reply->error().
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();   // 0 if absent
    if (m_timedOut || status <= 0) {
        finish(AuthResult::failure(AuthError::SERVER_UNREACHABLE), status);
        return;
    }

    const int retryAfter = parseRetryAfter(reply->rawHeader("Retry-After"));
    const QByteArray body = reply->readAll();
    finish(AuthJsonMapper::parseLoginResponse(status, retryAfter, body, domain::TimeStamp::now()), status);
}

void HttpsSessionGateway::finish(const AuthResult& result, int httpStatus)
{
    // Allowed in the log: request id, status, name of the error, duration. NEVER body, password or token.
    qInfo().noquote() << "[gateway] login request_id=" << m_requestId
                      << "status=" << httpStatus
                      << "result=" << (result.ok() ? QStringLiteral("OK")
                                                   : QString::fromStdString(application::toString(result.error())))
                      << "duration_ms=" << m_elapsed.elapsed();
    emit loginFinished(result);
}

} // namespace infrastructure
