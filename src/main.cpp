#include "event_queue.hpp"
#include "logger.hpp"

#include <iostream>

int main() {
    EventQueue queue;
    
    Event event{
        .type = EventType::MotionDetected,
        .source = "fake_sensor",
        .timestamp = std::chrono::system_clock::now(),
        .payload = "Motion detected"
    };

    queue.push(event);

    Logger logs{std::cout};

    logs.log(event);

    std::cout << event.source << ": "
              << event.payload << '\n';

    return 0;
}