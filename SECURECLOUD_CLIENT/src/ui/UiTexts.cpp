#include "ui/UiTexts.h"

#include <QCoreApplication>

namespace ui {

namespace {

// Gives us a tr() function that lupdate understands, without needing a QObject.
class Texts
{
    Q_DECLARE_TR_FUNCTIONS(Texts)
};

} // namespace

QString authErrorMessage(application::AuthError error, int retryAfterSeconds)
{
    using application::AuthError;

    // No "default:" on purpose: if someone adds an AuthError value, the
    // compiler warns (-Wswitch) and reminds us to add its sentence here.
    switch (error) {
    case AuthError::INVALID_INPUT:
        return Texts::tr("Username or password is missing or malformed.");
    case AuthError::INVALID_CREDENTIALS:
        return Texts::tr("Incorrect username or password.");
    case AuthError::ACCOUNT_DISABLED:
        return Texts::tr("This account is disabled. Please contact your administrator.");
    case AuthError::RATE_LIMITED:
        if (retryAfterSeconds > 0) {
            // %n gives correct singular/plural forms in every language.
            return Texts::tr("Too many attempts. Try again in %n second(s).",
                             nullptr, retryAfterSeconds);
        }
        return Texts::tr("Too many attempts. Please try again later.");
    case AuthError::SERVER_UNREACHABLE:
        return Texts::tr("Cannot reach the server. Check your connection and try again.");
    case AuthError::SERVICE_UNAVAILABLE:
        return Texts::tr("The service is temporarily unavailable. Please try again later.");
    case AuthError::INVALID_RESPONSE:
        return Texts::tr("Unexpected response from the server.");
    }
    return Texts::tr("Unexpected error.");   // value outside the enum (should never happen)
}

bool isTransientError(application::AuthError error)
{
    using application::AuthError;
    return error == AuthError::RATE_LIMITED
        || error == AuthError::SERVER_UNREACHABLE
        || error == AuthError::SERVICE_UNAVAILABLE;
}

QString roleDisplayName(domain::UserRole role)
{
    using domain::UserRole;

    switch (role) {
    case UserRole::ADMIN:    return Texts::tr("Administrator");
    case UserRole::COORD:    return Texts::tr("Coordinator");
    case UserRole::STD_USER: return Texts::tr("User");
    }
    return Texts::tr("Unknown");
}

} // namespace ui
