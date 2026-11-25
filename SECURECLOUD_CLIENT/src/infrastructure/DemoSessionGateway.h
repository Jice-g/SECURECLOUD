/**
 * @file DemoSessionGateway.h
 * @brief DEMO ONLY - scripted ISessionGateway so the UI can be run and tried
 *        without any server (the real HttpsSessionGateway is built in another thread).
 *
 * It does NO network, NO cryptography, checks NO real password. It exists to
 * exercise every screen state of UC1 by typing a magic username.
 * It is compiled only when the CMake option SC_DEMO_GATEWAY is ON, and it
 * must never be part of a release build.
 *
 * No Qt here: it is plain C++20 like the Application layer it plugs into.
 * login() runs in the worker thread, so sleeping is harmless for the UI.
 *
 * ADAPT-ME (the only guesses of this file): the constructors of domain::UserId and
 * domain::User, and TimeStamp::now(), are used as described in UC1_UML.md.
 * If your real signatures differ, fix makeSession() below, nothing else.
 */
#ifndef SC_INFRASTRUCTURE_DEMOSESSIONGATEWAY_H
#define SC_INFRASTRUCTURE_DEMOSESSIONGATEWAY_H

#include <chrono>
#include <string>
#include <thread>
#include <utility>

#include "application/loginusecase/AuthError.h"
#include "application/loginusecase/AuthResult.h"
#include "application/loginusecase/AuthSession.h"
#include "application/gateway/ISessionGateway.h"
#include "application/loginusecase/LoginProcessDTO.h"
#include "domain/model/Enums.h"
#include "domain/model/TimeStamp.h"
#include "domain/model/User.h"
#include "domain/model/UserId.h"

namespace infrastructure {

/**
 * @brief Magic usernames (case-insensitive, UserLogin lower-cases them):
 *
 *  | username   | result                                   |
 *  |------------|------------------------------------------|
 *  | alice      | success, role STD_USER                   |
 *  | coord      | success, role COORD                      |
 *  | admin      | success, role ADMIN                      |
 *  | disabled   | ACCOUNT_DISABLED                         |
 *  | ratelimit  | RATE_LIMITED, retry after 10 s           |
 *  | offline    | SERVER_UNREACHABLE                       |
 *  | down       | SERVICE_UNAVAILABLE                      |
 *  | garbage    | INVALID_RESPONSE                         |
 *  | (other)    | INVALID_CREDENTIALS                      |
 *
 * For alice/coord/admin the password must be DEMO_PASSWORD, otherwise INVALID_CREDENTIALS.
 */
class DemoSessionGateway final : public application::ISessionGateway
{
public:
    static constexpr const char* DEMO_PASSWORD = "azerty";
    static constexpr int LATENCY_MS = 700;           // makes the busy state visible
    static constexpr int RATE_LIMIT_SECONDS = 10;

    [[nodiscard]]
    application::AuthResult login(const application::LoginRequestDTO& request) override
    {
        using application::AuthError;
        using application::AuthResult;

        std::this_thread::sleep_for(std::chrono::milliseconds(LATENCY_MS));

        const std::string name = request.login.value();

        if (name == "offline")   { return AuthResult::failure(AuthError::SERVER_UNREACHABLE); }
        if (name == "down")      { return AuthResult::failure(AuthError::SERVICE_UNAVAILABLE); }
        if (name == "garbage")   { return AuthResult::failure(AuthError::INVALID_RESPONSE); }
        if (name == "disabled")  { return AuthResult::failure(AuthError::ACCOUNT_DISABLED); }
        if (name == "ratelimit") {
            return AuthResult::failure(AuthError::RATE_LIMITED, RATE_LIMIT_SECONDS);
        }

        domain::UserRole role = domain::UserRole::STD_USER;
        if (name == "admin")      { role = domain::UserRole::ADMIN; }
        else if (name == "coord") { role = domain::UserRole::COORD; }
        else if (name != "alice") { return AuthResult::failure(AuthError::INVALID_CREDENTIALS); }

        if (request.password != DEMO_PASSWORD) {
            return AuthResult::failure(AuthError::INVALID_CREDENTIALS);
        }
        return AuthResult::success(makeSession(request, role));
    }

private:
    static application::AuthSession makeSession(const application::LoginRequestDTO& request,
                                                domain::UserRole role)
    {
        // ADAPT-ME if your domain constructors differ (see the file header).
        domain::User user(domain::UserId("demo-" + request.login.value()),
                          request.login,
                          role,
                          domain::TimeStamp::now());
        return application::AuthSession("demo-access-token",
                                        "demo-refresh-token",
                                        domain::TimeStamp::now(),   // expiry is irrelevant in the demo
                                        std::move(user));
    }
};

} // namespace infrastructure

#endif // SC_INFRASTRUCTURE_DEMOSESSIONGATEWAY_H
