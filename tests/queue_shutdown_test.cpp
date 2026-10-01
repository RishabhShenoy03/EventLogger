#include <cassert>
#include <sstream>
#include <array>
#include <thread>

#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"

int main() {

    EventQueue queue{};
    
    std::jthread main_thread([&queue]{
        Event event_a{EventType::MotionDetected, "FakeSensor", std::chrono::system_clock::time_point{
            std::chrono::seconds{10}}, "Event A"};
        Event event_b{EventType::SensorReading, "FakeSensor", std::chrono::system_clock::time_point{
            std::chrono::seconds{20}}, "Event B"};
        Event event_c{EventType::DeviceOnline, "FakeSensor", std::chrono::system_clock::time_point{
            std::chrono::seconds{30}}, "Event C"};
        Event event_d{EventType::DeviceOnline, "FakeSensor", std::chrono::system_clock::time_point{
            std::chrono::seconds{40}}, "Event D"};
    
        queue.push(event_a);
        queue.push(event_b);
        queue.push(event_c);
        queue.push(event_d);

        

    });
}