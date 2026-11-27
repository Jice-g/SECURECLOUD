#pragma once

#include <QList>
#include <QSslCertificate>
#include <QString>
#include <QUrl>

/// @file GatewayConfig.h


/* ***
 * In the main() :
 *  ---
 *     const auto env = QProcessEnvironment::systemEnvironment();
 *     infrastructure::GatewayConfig config;
 *
 *  // Create the keys :
 *      config.loginUrl  = QUrl(env.value(QStringLiteral("SC_GATEWAY_URL"), QStringLiteral("https://localhost:8443/auth/login")));
 *      config.caCertPath = env.value(QStringLiteral("SC_DEV_CA"), QStringLiteral("dev-ca.pem"));
 *
 *  // then this config is injected in the gateway instance :
 *      infrastructure::HttpsSessionGateway gateway(config);
 *  ---
 * ***/

namespace infrastructure {

/// @brief Settings of the HTTPS gateway, filled by main.cpp from the environment (ADR UC1-19).
/// @details A plain value: no behaviour but a check. main.cpp calls validate() at start-up and quits
///          with a clear message if it fails; HttpsSessionGateway checks it again (defence in depth).
///
struct GatewayConfig {
    QUrl    loginUrl;                 ///< exemple : https://localhost:8443/auth/login.
    QString caCertPath;               ///< Optional dev CA (PEM file), ADDED to the system CAs. Empty = none.
    int     totalTimeoutMs = 20000;   ///< Total time allowed for one login (connection + answer), in ms.

    /// @brief Checks the settings.
    /// @return an empty string if everything is fine, otherwise the REASON (for the start-up error message).
    ///         The checks: valid https URL with a host, timeout > 0, CA file readable and holding at least
    ///         one PEM certificate (only if caCertPath is not empty).
    ///
    [[nodiscard]]
    QString validate() const;

    /// @brief Reads the certificates of the dev CA file.
    /// @return the certificates, or an empty list if caCertPath is empty or the file is unusable.
    ///
    [[nodiscard]]
    QList<QSslCertificate> loadCaCertificates() const;
};

} // namespace infrastructure
