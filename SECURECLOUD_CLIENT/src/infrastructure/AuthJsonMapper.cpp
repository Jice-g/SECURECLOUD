#include "infrastructure/AuthJsonMapper.h"

#include <chrono>
#include <stdexcept>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace infrastructure {

using application::AuthError;
using application::AuthResult;
using application::AuthSession;

namespace {

// The names of the contract (snake_case)
// this the name of the KEY in the JSON file
// this is the place where they are written, and then we can use these constances
const QLatin1String KEY_LOGIN("login");
const QLatin1String KEY_PASSWORD("password");
const QLatin1String KEY_MFA_CODE("mfa_code");
const QLatin1String KEY_ACCESS_TOKEN("access_token");
const QLatin1String KEY_REFRESH_TOKEN("refresh_token");
const QLatin1String KEY_EXPIRES_IN("expires_in");
const QLatin1String KEY_USER("user");
const QLatin1String KEY_USER_ID("id");
const QLatin1String KEY_USER_LOGIN("login");
const QLatin1String KEY_USER_ROLE("role");


/* **********************************************************
 *  usefull function to manage the JsonObject
 *  they will be used in the methods of the mapper class
 *  ********************************************************* */

AuthResult invalidResponse()
{
    return AuthResult::failure(AuthError::INVALID_RESPONSE);
}

/// Status 200: read the body. Any missing, empty or badly typed field => INVALID_RESPONSE.
AuthResult parseSuccess(const QByteArray& body, const domain::TimeStamp& now)
{
    // extracting data :
    const QJsonObject root = QJsonDocument::fromJson(body).object();   // not JSON => empty object => fails below
    const QJsonObject userObject = root.value(KEY_USER).toObject();

    // toString() gives "" if the value is missing or not a string: so "empty" covers all the bad cases.
    const QString accessToken  = root.value(KEY_ACCESS_TOKEN).toString();
    const QString refreshToken = root.value(KEY_REFRESH_TOKEN).toString();
    const int     expiresIn    = root.value(KEY_EXPIRES_IN).toInt();   // 0 if missing or not a number
    const QString userId       = userObject.value(KEY_USER_ID).toString();
    const QString userLogin    = userObject.value(KEY_USER_LOGIN).toString();
    const QString roleName     = userObject.value(KEY_USER_ROLE).toString();

    if (accessToken.isEmpty() || refreshToken.isEmpty() || expiresIn <= 0 || userId.isEmpty() || userLogin.isEmpty())
    {
        return invalidResponse();
    }

    // An unknown role is a failure: NEVER a default role.
    domain::UserRole role;
    if (roleName == QLatin1String("ADMIN"))
        { role = domain::UserRole::ADMIN; }
    else if (roleName == QLatin1String("COORD"))
        { role = domain::UserRole::COORD; }
    else if (roleName == QLatin1String("STD_USER"))
        { role = domain::UserRole::STD_USER; }
    else
        { return invalidResponse(); }

    // The Domain checks its own rules (login format...) and throws std::invalid_argument.
    try {
        const domain::User user( domain::UserId(userId.toStdString()),
                                 domain::UserLogin(userLogin.toStdString()),
                                 role, now);   // created_at is not read: the connection time is used (now)
        const domain::TimeStamp expiresAt(now.value() + std::chrono::seconds(expiresIn));

        return AuthResult::success(AuthSession(accessToken.toStdString(), refreshToken.toStdString(), expiresAt, user));
        // the domain rules had been defined in that purpose
    } catch (const std::invalid_argument&) {
        return invalidResponse();
    }
}

} // namespace

/* *******************************************************
 *    Implement the methods of the json mapper
 * ******************************************************* */

QByteArray AuthJsonMapper::buildLoginBody(const application::LoginRequestDTO& request)
{
    QJsonObject object;
    object.insert(KEY_LOGIN, QString::fromStdString(request.login.value()));
    object.insert(KEY_PASSWORD, QString::fromUtf8(request.password.data(),
                                                  static_cast<qsizetype>(request.password.size())));
    object.insert(KEY_MFA_CODE, QJsonValue(QJsonValue::Null));   // reserved: null in v0
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

AuthResult AuthJsonMapper::parseLoginResponse(int httpStatus, int retryAfterSeconds,
                                              const QByteArray& body, const domain::TimeStamp& now)
{
    // One answer per HTTP status. Only the success (200) needs the body.
    switch (httpStatus) {
    case 200: return parseSuccess(body, now);
    case 400:
    case 413: return AuthResult::failure(AuthError::INVALID_INPUT);
    case 401: return AuthResult::failure(AuthError::INVALID_CREDENTIALS);
    case 403: return AuthResult::failure(AuthError::ACCOUNT_DISABLED);
    case 429: return AuthResult::failure(AuthError::RATE_LIMITED, retryAfterSeconds);
    case 500:
    case 502:
    case 503:
    case 504: return AuthResult::failure(AuthError::SERVICE_UNAVAILABLE);
    default:  return invalidResponse();   // 3xx, 415, other 2xx/4xx/5xx...
    }
}

} // namespace infrastructure
