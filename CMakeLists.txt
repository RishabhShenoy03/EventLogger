cmake_minimum_required(VERSION 3.16)

project(EventLogger LANGUAGES CXX)

enable_testing()

add_executable(event_gateway
    src/main.cpp
    src/event_queue.cpp
)

add_executable(event_queue_test
    tests/event_queue_test.cpp
    src/event_queue.cpp
)

target_compile_features(event_gateway PRIVATE cxx_std_23)
target_compile_features(event_queue_test PRIVATE cxx_std_23)

target_compile_options(event_gateway PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_options(event_queue_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_include_directories(event_gateway PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(event_queue_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(
    NAME event_queue_test
    COMMAND event_queue_test
)