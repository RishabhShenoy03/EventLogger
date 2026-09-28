#include <cassert>

#include "event_queue.hpp"

int main() {
    EventQueue queue{};

    assert(queue.empty());
    Event A(EventType::MotionDetected, "Event A", std::chrono::::system_clock::now(), "Motion Detected");
    Event B(EventType::SensorReading, "Event B", std::chrono::::system_clock::now(), "Sensor Reading");
    Event C(EventType::DeviceOnline, "Event C", std::chrono::::system_clock::now(), "Device Online");
    queue.push(A);
    queue.push(B);
    queue.push(C);

    assert(queue.size() == 3);



}