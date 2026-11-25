/**
 * @file UiTexts.h
 * @brief User-facing texts for UC1, kept in ONE place and wrapped in tr()
 *        so they can be translated later (lupdate / Qt Linguist).
 *
 * Why here and not in the windows? The mapping "AuthError -> sentence" is
 * a presentation decision (UI layer). The Application layer only returns codes.
 */
#ifndef SC_UI_UITEXTS_H
#define SC_UI_UITEXTS_H

#include <QString>

#include "application/loginusecase/AuthError.h"
#include "domain/model/Enums.h"

namespace ui {

/**
 * @brief Sentence shown to the user for a login failure.
 *
 * "Unknown login" and "wrong password" share INVALID_CREDENTIALS and therefore
 * the same sentence (ADR UC1-10): never reveal whether an account exists.
 *
 * @param error             Code returned by the use case.
 * @param retryAfterSeconds Delay sent by the server (only used for RATE_LIMITED).
 */
QString authErrorMessage(application::AuthError error, int retryAfterSeconds);

/**
 * @brief True if retrying later may work (network, server load, rate limit).
 *        The window shows such errors as a warning (amber) instead of an error (red).
 */
bool isTransientError(application::AuthError error);

/**
 * @brief Human-readable role name. Display only: never a security barrier.
 */
QString roleDisplayName(domain::UserRole role);

} // namespace ui

#endif // SC_UI_UITEXTS_H
