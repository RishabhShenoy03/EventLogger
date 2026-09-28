#include <cassert>

#include "event_queue.hpp"

int main() {
    EventQueue queue{};

    assert(queue.empty());
    
    Event A(EventType::MotionDetected, "EventQueue Test", std::chrono::system_clock::now(), "Motion Detected");
    Event B(EventType::SensorReading, "EventQueue Test", std::chrono::system_clock::now(), "Sensor Reading");
    Event C(EventType::DeviceOnline, "EventQueue Test", std::chrono::system_clock::now(), "Device Online");
    queue.push(A);
    queue.push(B);
    queue.push(C);
    assert(queue.size() == 3);

    auto first = queue.try_pop();
    assert(first.has_value());
    assert(first->source == "Event A");
    assert(queue.size() == 2);

    auto second = queue.try_pop();
    assert(second.has_value());
    assert(second->source == "Event B");
    assert(queue.size() == 1);

    auto third = queue.try_pop();
    assert(third.has_value());
    assert(third->source == "Event C");
    assert(queue.size() == 0);

    auto empty_pop = queue.try_pop();
    assert(queue.empty());
    assert(empty_pop == std::nullopt);
    assert(!empty_pop.has_value());
}