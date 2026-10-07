cmake_minimum_required(VERSION 3.16)

project(EventLogger LANGUAGES CXX)

enable_testing()

find_package(OpenCV REQUIRED COMPONENTS
    core
    imgcodecs
    imgproc
    videoio
    highgui
    objdetect
    dnn
)

# Reusable project code
add_library(event_core
    src/event_queue.cpp
    src/logger.cpp
    src/fake_sensor.cpp
    src/CameraCapture.cpp
    src/FaceProcessor.cpp
    src/FaceRegistry.cpp
)

target_compile_features(event_core PUBLIC cxx_std_23)

target_include_directories(event_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}

)
target_link_libraries(event_core PUBLIC ${OpenCV_LIBS})

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
## Automated tests

add_executable(pipeline_test
    tests/pipeline_test.cpp
)
target_link_libraries(pipeline_test PRIVATE event_core)
add_test(NAME pipeline_test COMMAND pipeline_test)

add_executable(event_queue_test
    tests/event_queue_test.cpp
)
target_link_libraries(event_queue_test PRIVATE event_core)
add_test(NAME event_queue_test COMMAND event_queue_test)

add_executable(logger_test
    tests/logger_test.cpp
)
target_link_libraries(logger_test PRIVATE event_core)
add_test(NAME logger_test COMMAND logger_test)

add_executable(fake_sensor_test
    tests/fake_sensor_test.cpp
)
target_link_libraries(fake_sensor_test PRIVATE event_core)
add_test(NAME fake_sensor_test COMMAND fake_sensor_test)

# Camera/model integration test.
# Build it with the project, but run it manually on the Pi because it needs
# a connected camera and the model files.
add_executable(camera_capture_test
    tests/camera_capture_test.cpp
)
target_link_libraries(camera_capture_test PRIVATE event_core)