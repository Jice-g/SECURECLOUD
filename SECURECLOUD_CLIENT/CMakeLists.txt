cmake_minimum_required(VERSION 3.21)
project(SecureCloud_Client VERSION 0.1 LANGUAGES CXX)

# --- Build options ---
option(SC_BUILD_UI  "Build the Qt GUI (needs Qt6 Widgets). OFF = no GUI; Qt Core/Network are still required" ON)
option(SC_GUI_ONLY  "Hide the console window on Windows (release/demo builds)" OFF)

# --- Global settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Qt is now a base dependency (ADR UC1-16/17): the Application layer uses Qt Core (QObject, signals),
# the Infrastructure layer uses Qt Core + Network. The Domain stays pure C++ (NO Qt).
# On Windows, the Qt DLLs must be reachable to RUN the tests (PATH, or windeployqt).
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network)

# Q_OBJECT classes live in Application and Infrastructure: moc must run on those targets.
# (Set BEFORE the add_library calls below: the property is read when a target is created.)
set(CMAKE_AUTOMOC ON)

# =============================================================================
# 1. Domain library (pure C++, NO Qt)
# =============================================================================
add_library(sc_domain STATIC
    #  Headers  ----
    src/domain/model/Enums.h
    src/domain/model/MessageContent.h
    src/domain/model/TimeStamp.h
    src/domain/model/User.h
    src/domain/model/UserId.h
    src/domain/model/UserLogin.h
    src/domain/model/Conversation.h
    src/domain/model/ConversationId.h
    src/domain/model/AttachmentId.h
    src/domain/model/Message.h
    src/domain/model/MessageId.h
    # Sources  ----
    src/domain/model/Message.cpp
    src/domain/model/UserLogin.cpp
)
# Headers are found through the include path (no need to list them here).
target_include_directories(sc_domain PUBLIC src)
# sc_domain links NO Qt module.

# =============================================================================
# 1b. Application library (C++20 + Qt Core only): use cases, ports, session state
# =============================================================================
find_package(Threads REQUIRED)   # SessionManager still uses std::mutex (removal: Roadmap)

add_library(sc_application STATIC
    #  Headers  ----
    src/application/loginusecase/AuthError.h
    src/application/loginusecase/AuthResult.h
    src/application/loginusecase/AuthSession.h
    src/application/loginusecase/LoginOutcome.h
    src/application/loginusecase/LoginProcessDTO.h
    src/application/loginusecase/LoginUseCase.h
    src/application/loginusecase/SessionManager.h
    src/application/gateway/ISessionGateway.h
    # Sources  ----
    src/application/loginusecase/LoginUseCase.cpp
    src/application/loginusecase/SessionManager.cpp
)
# The Application layer may use Qt Core, NEVER Widgets or Gui (ADR UC1-17).
# sc_domain is PUBLIC so that whoever links sc_application also gets the domain headers (and `src` include path).
target_link_libraries(sc_application PUBLIC sc_domain Qt6::Core Threads::Threads)

# =============================================================================
# 1c. Infrastructure library (Qt Core + Network, NO Widgets): adapters of the ports
#     Built without the GUI, so that the mapper / gateway tests (S1b, S2) need no Widgets.
#     The files arrive step by step (S1a: mapper, S3b: config + gateway): the target is
#     built from the files that are already in the source tree.
# =============================================================================
set(SC_INFRA_SOURCES
    src/infrastructure/AuthJsonMapper.h
    src/infrastructure/AuthJsonMapper.cpp
    src/infrastructure/GatewayConfig.h
    src/infrastructure/GatewayConfig.cpp
    src/infrastructure/HttpsSessionGateway.h
    src/infrastructure/HttpsSessionGateway.cpp
)
# Only the files that exist are compiled: the target grows with the steps (S1a, then S3b).
set(SC_INFRA_PRESENT "")
set(SC_INFRA_COMPLETE TRUE)
foreach(f IN LISTS SC_INFRA_SOURCES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
        list(APPEND SC_INFRA_PRESENT ${f})
    else()
        set(SC_INFRA_COMPLETE FALSE)
    endif()
endforeach()

if(SC_INFRA_PRESENT)
    add_library(sc_infrastructure STATIC ${SC_INFRA_PRESENT})
    target_link_libraries(sc_infrastructure PUBLIC sc_application Qt6::Core Qt6::Network)
endif()
if(NOT SC_INFRA_COMPLETE)
    message(STATUS "sc_infrastructure: not all sources are present yet (steps S1a/S3b).")
endif()

# =============================================================================
# 2. Domain tests (console only, no Qt) -- built in every configuration
# =============================================================================
enable_testing()


add_executable(run_domain_tests
    tests/domain_tests/domain_test_1.cpp
    tests/harness_test.h
)

target_link_libraries(run_domain_tests PRIVATE sc_domain)

target_include_directories(run_domain_tests PRIVATE tests)

add_test(NAME DomainTests COMMAND run_domain_tests ${CMAKE_BINARY_DIR}/domain_report.txt)


# =============================================================================
# 2b. Application tests (console only, no network, no event loop) -- built in every configuration
#     They link sc_application, hence Qt Core (QObject, signals): no QCoreApplication needed because
#     the fakes answer synchronously.
#     Two executables: SessionManager has its own because it is also run under -fsanitize=thread.
# =============================================================================
add_executable(run_application_tests
    tests/application_tests/application_test_1.cpp
    tests/application_tests/fakes/FakeSessionGateway.h
    tests/harness_test.h
)
target_link_libraries(run_application_tests PRIVATE sc_application)
target_include_directories(run_application_tests PRIVATE tests)
add_test(NAME ApplicationTests COMMAND run_application_tests ${CMAKE_BINARY_DIR}/application_report.txt)

add_executable(run_session_manager_tests
    tests/application_tests/session_manager_test.cpp
    tests/harness_test.h
)
target_link_libraries(run_session_manager_tests PRIVATE sc_application)
target_include_directories(run_session_manager_tests PRIVATE tests)
add_test(NAME SessionManagerTests COMMAND run_session_manager_tests ${CMAKE_BINARY_DIR}/session_manager_report.txt)



# =============================================================================
# 3. Qt GUI (optional)
# =============================================================================
if(SC_BUILD_UI)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

    if(NOT SC_INFRA_COMPLETE)
        message(FATAL_ERROR "The GUI needs sc_infrastructure (src/infrastructure/*): steps S1a/S3b are not in the source tree yet. Configure with -DSC_BUILD_UI=OFF to build the tests only.")
    endif()

    # UC1 demo backend: a scripted, asynchronous ISessionGateway (no network, no real authentication)
    # so the UI can be tried without a server. OFF by default: the client uses HttpsSessionGateway.
    # NEVER ship ON.
    option(SC_DEMO_GATEWAY "Use the scripted DemoSessionGateway instead of the HTTPS gateway (DEMO ONLY)" OFF)

    qt_add_executable(SecureCloud_Client
        src/main.cpp
        src/ui/Theme.cpp
        src/ui/Theme.h
        src/ui/UiTexts.cpp
        src/ui/UiTexts.h
        src/ui/LoginController.cpp
        src/ui/LoginController.h
        src/ui/UserLoginWindow.cpp
        src/ui/UserLoginWindow.h
        src/ui/UserLoginWindow.ui
        src/ui/ClientChatWindow.cpp
        src/ui/ClientChatWindow.h
        src/ui/ClientChatWindow.ui
        src/infrastructure/DemoSessionGateway.h
    )

    # The UI knows the Application layer (use case, session), the Domain, and (main.cpp only) the Infrastructure.
    target_link_libraries(SecureCloud_Client PRIVATE sc_infrastructure sc_application sc_domain Qt6::Widgets)

    if(SC_DEMO_GATEWAY)
        target_compile_definitions(SecureCloud_Client PRIVATE SC_DEMO_GATEWAY=1)
        message(WARNING "SC_DEMO_GATEWAY is ON: the client uses a FAKE login backend (demo only).")
    endif()

    set_target_properties(SecureCloud_Client PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        WIN32_EXECUTABLE ${SC_GUI_ONLY}
    )

    include(GNUInstallDirs)
    install(TARGETS SecureCloud_Client
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
