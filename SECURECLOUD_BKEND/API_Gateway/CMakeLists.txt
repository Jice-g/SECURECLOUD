cmake_minimum_required(VERSION 3.20)

project(ApiGateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Crow CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_executable(ApiGateway
    src/Main.cpp
    Src/AuthClient.cpp
    Src/AuthFilter.cpp
    Src/RouteurTest.cpp
    src/generated/auth.grpc.pb.cc
    src/generated/auth.pb.cc
    src/generated/messaging.grpc.pb.cc
    src/generated/messaging.pb.cc
)

target_include_directories(ApiGateway PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generated"
)

target_link_libraries(ApiGateway PRIVATE
    Crow::Crow
    gRPC::grpc++
    protobuf::libprotobuf
)

if(MSVC)
    target_compile_definitions(ApiGateway PRIVATE _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0A00)
    target_compile_options(ApiGateway PRIVATE /wd4996)
endif()