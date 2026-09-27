cmake_minimum_required(VERSION 3.16)

project(EventLogger LANGUAGES CXX)

add_executable(event_gateway
    src/main.cpp
)

target_compile_features(event_gateway PRIVATE cxx_std_23)

target_compile_options(event_gateway PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_include_directories(event_gateway PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)