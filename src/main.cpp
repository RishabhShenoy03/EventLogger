#include "event.hpp"

#include <iostream>

int main() {
    Event event{
        .type = EventType::MotionDetected,
        .source = "fake_sensor",
        .timestamp = std::chrono::system_clock::now(),
        .payload = "Motion detected"
    };

    std::cout << event.source << ": "
              << event.payload << '\n';

    return 0;
}