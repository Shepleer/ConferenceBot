cmake_minimum_required(VERSION 3.20)

project(
    ConferenceBot
    VERSION 1.0
    DESCRIPTION "A bot for managing conference events"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Drogon CONFIG REQUIRED)
find_package(TgBot CONFIG REQUIRED)

file(
    GLOB_RECURSE
    SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(
    ConferenceBot
    ${SOURCES}
    models/Registrations.cc
)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

target_compile_definitions(
    ConferenceBot
    PRIVATE
    ASSETS_DIR="${ASSETS_DIR}"
)

target_include_directories(
    ConferenceBot
    PRIVATE
    include
    models
)

target_compile_features(
    ConferenceBot
    PRIVATE
    cxx_std_26
)


target_compile_options(
    ConferenceBot
    PRIVATE
    -g
)

target_link_libraries(
    ConferenceBot
    PRIVATE
    Drogon::Drogon
    TgBot::TgBot
)