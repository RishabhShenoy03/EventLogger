cmake_minimum_required(VERSION 3.16)

project(EventLogger LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

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

find_package(TgBot CONFIG REQUIRED)
add_executable(bot_example main.cpp)
target_link_libraries(bot_example PRIVATE TgBot::TgBot)

# Reusable project code
add_library(event_core
    src/EventQueue.cpp
    src/Logger.cpp
    src/CameraCapture.cpp
    src/FaceProcessor.cpp
    src/FaceRegistry.cpp
    src/TelegramClient.cpp
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

# add_executable(pipeline_test
#     tests/pipeline_test.cpp
# )
# target_link_libraries(pipeline_test PRIVATE event_core)
# add_test(NAME pipeline_test COMMAND pipeline_test)

# add_executable(EventQueue_test
#     tests/EventQueue_test.cpp
# )
# target_link_libraries(EventQueue_test PRIVATE event_core)
# add_test(NAME EventQueue_test COMMAND EventQueue_test)

# add_executable(Logger_test
#     tests/Logger_test.cpp
# )
# target_link_libraries(Logger_test PRIVATE event_core)
# add_test(NAME Logger_test COMMAND Logger_test)

# add_executable(fake_sensor_test
#     tests/fake_sensor_test.cpp
# )
# target_link_libraries(fake_sensor_test PRIVATE event_core)
# add_test(NAME fake_sensor_test COMMAND fake_sensor_test)

# Camera/model integration test.
# Build it with the project, but run it manually on the Pi because it needs
# a connected camera and the model files.
add_executable(camera_capture_test
    tests/camera_capture_test.cpp
)
target_link_libraries(camera_capture_test PRIVATE event_core)