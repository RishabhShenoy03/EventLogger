#include <cassert>

#include "event_queue.hpp"

int main() {
    EventQueue queue{};

    assert(queue.empty());
    
    Event event_a{EventType::MotionDetected, "Event A", std::chrono::system_clock::time_point{
        std::chrono::seconds{1}}, "Motion Detected"};
    Event event_b{EventType::SensorReading, "Event B", std::chrono::system_clock::time_point{
        std::chrono::seconds{1}}, "Sensor Reading"};
    Event event_c{EventType::DeviceOnline, "Event C", std::chrono::system_clock::time_point{
        std::chrono::seconds{3}}, "Device Online"};
    queue.push(event_a);
    queue.push(event_b);
    queue.push(event_c);
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