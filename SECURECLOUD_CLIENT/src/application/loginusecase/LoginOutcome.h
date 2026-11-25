#pragma once

#include <optional>
#include <string>
#include <utility>

#include "application/loginusecase/AuthError.h"
#include "domain/model/Enums.h"
#include "application/loginusecase/LoginProcessDTO.h"

/// @file LoginOutcome.h

namespace application {

/// @brief Return value of LoginUseCase::execute: EITHER a LoginResultDTO OR an AuthError.
/// @details Same shape as AuthResult but a different content (no token).
///
struct LoginOutcome {

    std::optional<LoginResultDTO> user;   ///< Set on success.
    std::optional<AuthError>      error;  ///< Set on failure.
    int retryAfterSeconds = 0;            ///< Meaningful for RATE_LIMITED.

    [[nodiscard]]
    bool ok() const noexcept
    { return user.has_value(); }

    [[nodiscard]]
    static LoginOutcome success(LoginResultDTO result)
    {
        LoginOutcome outcome;
        outcome.user = std::move(result);
        return outcome;
    }

    [[nodiscard]]
    static LoginOutcome failure(AuthError error, int retryAfterSeconds = 0)
    {
        LoginOutcome outcome;
        outcome.error = error;
        outcome.retryAfterSeconds = retryAfterSeconds;
        return outcome;
    }

};

} // namespace application
