cmake_minimum_required(VERSION 3.16)

project(EventLogger LANGUAGES CXX)

enable_testing()

find_package(OpenCV REQUIRED COMPONENTS
    core imgcodecs imgproc videoio highgui
)

# Reusable project code
add_library(event_core
    src/event_queue.cpp
    src/logger.cpp
    src/fake_sensor.cpp
)

target_compile_features(event_core PUBLIC cxx_std_23)
target_include_directories(event_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(event_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Main application
add_executable(event_gateway
    src/main.cpp
)

target_link_libraries(event_gateway PRIVATE
    event_core
)

add_executable(cameraaa src/camera_capture.cpp)
target_link_libraries(cameraaa PRIVATE ${OpenCV_LIBS})
target_include_directories(cameraaa PRIVATE ${OpenCV_INCLUDE_DIRS})

add_executable(file_gateway
    src/file_everything.cpp
)
target_link_libraries(file_gateway PRIVATE 
    event_core
)

## Tests ##

# Pipeline (whole) Test
add_executable(pipeline_test
    tests/pipeline_test.cpp
)

target_link_libraries(pipeline_test PRIVATE
    event_core
)

add_test(
    NAME pipeline_test
    COMMAND pipeline_test
)

# Event Queue Test
add_executable(event_queue_test
    tests/event_queue_test.cpp
)

target_link_libraries(event_queue_test PRIVATE
    event_core
)

add_test(
    NAME event_queue_test
    COMMAND event_queue_test
)

# Logger Test
add_executable(logger_test
    tests/logger_test.cpp
)

target_link_libraries(logger_test PRIVATE
    event_core
)

add_test(
    NAME logger_test
    COMMAND logger_test
)

# Fake Sensor Test
add_executable(fake_sensor_test
    tests/fake_sensor_test.cpp
)

target_link_libraries(fake_sensor_test PRIVATE
    event_core
)

add_test(
    NAME fake_sensor_test
    COMMAND fake_sensor_test
)
