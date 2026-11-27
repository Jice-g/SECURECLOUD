cmake_minimum_required(VERSION 3.20)

project(ApiGateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Crow CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(ApiGateway
    Src/Main.cpp
    Src/AuthClient.cpp
    Src/AuthFilter.cpp
    Src/RouteurTest.cpp
    Src/generated/auth.grpc.pb.cc
    Src/generated/auth.pb.cc
    Src/generated/messaging.grpc.pb.cc
    Src/generated/messaging.pb.cc
)

add_executable(AuthServiceMock
    tests/AuthServiceMock.cpp
    Src/generated/auth.grpc.pb.cc
    Src/generated/auth.pb.cc
)

add_executable(ApiGatewayTests
    tests/TestMain.cpp
    tests/AuthClientTest.cpp
    tests/AuthFilterTest.cpp
    tests/MessageParserTest.cpp
    tests/MessagingClientTest.cpp
    tests/WssControllerTest.cpp
    Src/AuthClient.cpp
    Src/AuthFilter.cpp
    Src/RouteurTest.cpp
    Src/generated/auth.grpc.pb.cc
    Src/generated/auth.pb.cc
    Src/generated/messaging.grpc.pb.cc
    Src/generated/messaging.pb.cc
)

target_include_directories(ApiGateway PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/generated"
)

target_include_directories(AuthServiceMock PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/generated"
)

target_include_directories(ApiGatewayTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/generated"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)

target_link_libraries(ApiGateway PRIVATE
    Crow::Crow
    gRPC::grpc++
    protobuf::libprotobuf
)

target_link_libraries(AuthServiceMock PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
)

target_link_libraries(ApiGatewayTests PRIVATE
    GTest::gtest
    Crow::Crow
    gRPC::grpc++
    protobuf::libprotobuf
)

if(MSVC)
    target_compile_definitions(ApiGateway PRIVATE _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0A00)
    target_compile_options(ApiGateway PRIVATE /wd4996)

    target_compile_definitions(AuthServiceMock PRIVATE _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0A00)
    target_compile_options(AuthServiceMock PRIVATE /wd4996)

    target_compile_definitions(ApiGatewayTests PRIVATE _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0A00)
    target_compile_options(ApiGatewayTests PRIVATE /wd4996)
endif()