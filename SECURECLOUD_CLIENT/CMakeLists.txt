cmake_minimum_required(VERSION 3.21)
project(SecureCloud_Client VERSION 0.1 LANGUAGES CXX)

# --- Build options ---
option(SC_BUILD_UI  "Build the Qt GUI (needs Qt6). Turn OFF to build/test the domain only (CI, servers)" ON)
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

# =============================================================================
# 2. Domain tests (console only, no Qt) -- built in every configuration
# =============================================================================
enable_testing()

add_executable(run_tests
	tests/domain_test_1.cpp
	tests/domain_test.h
	)
	
target_link_libraries(run_tests PRIVATE sc_domain)

target_include_directories(run_tests PRIVATE tests)

add_test(NAME DomainTests COMMAND run_tests ${CMAKE_BINARY_DIR}/domain_report.txt)

# =============================================================================
# 3. Qt GUI (optional)
# =============================================================================
if(SC_BUILD_UI)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

    qt_add_executable(SecureCloud_Client
        src/main.cpp
        src/ui/clientchatwindow.cpp
        src/ui/clientchatwindow.h
        src/ui/clientchatwindow.ui
    )
	
    target_link_libraries(SecureCloud_Client PRIVATE sc_domain Qt6::Widgets)

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
