#include "infrastructure/GatewayConfig.h"

#include <QFile>
#include <QLatin1String>

namespace infrastructure {

namespace {

/// Reads a PEM file. @return "" if it worked (certificates in `certificates`), otherwise the reason.
QString readCaFile(const QString& path, QList<QSslCertificate>& certificates)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QStringLiteral("the CA file cannot be read: ") + path;
    }
    certificates = QSslCertificate::fromData(file.readAll(), QSsl::Pem);
    if (certificates.isEmpty()) {
        return QStringLiteral("the CA file holds no PEM certificate: ") + path;
    }
    return QString();
}

} // namespace

QString GatewayConfig::validate() const
{
    // The URL is shown in the message without any user info (it must never carry a secret).
    const QString shownUrl = loginUrl.toString(QUrl::RemoveUserInfo);

    if (!loginUrl.isValid()) {
        return QStringLiteral("the gateway URL is not valid: ") + shownUrl;
    }
    if (loginUrl.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) != 0) {
        return QStringLiteral("the gateway URL must use https (plain http is refused): ") + shownUrl;
    }
    if (loginUrl.host().isEmpty()) {
        return QStringLiteral("the gateway URL has no host name: ") + shownUrl;
    }
    if (totalTimeoutMs <= 0) {
        return QStringLiteral("the timeout must be greater than 0 ms");
    }
    if (!caCertPath.isEmpty()) {
        QList<QSslCertificate> certificates;
        const QString reason = readCaFile(caCertPath, certificates);
        if (!reason.isEmpty()) {
            return reason;
        }
    }
    return QString();   // everything is fine
}

QList<QSslCertificate> GatewayConfig::loadCaCertificates() const
{
    QList<QSslCertificate> certificates;
    if (!caCertPath.isEmpty() && !readCaFile(caCertPath, certificates).isEmpty()) {
        certificates.clear();   // unusable file: nothing is added
    }
    return certificates;
}

} // namespace infrastructure
