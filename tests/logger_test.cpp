#include <cassert>
#include <chrono>
#include <sstream>

#include "logger.hpp"

int main() {
    std::ostringstream captured_output;
    Logger logs{captured_output};

    Event event_a{EventType::MotionDetected, "LoggerTest", std::chrono::system_clock::time_point{
        std::chrono::seconds{1}}, "Event A"};
    Event event_b{EventType::SensorReading, "LoggerTest", std::chrono::system_clock::time_point{
        std::chrono::seconds{2}}, "Event B"};
    Event event_c{EventType::DeviceOnline, "LoggerTest", std::chrono::system_clock::time_point{
        std::chrono::seconds{3}}, "Event C"};
    
    logs.log(event_a);
    logs.log(event_b);
    logs.log(event_c);

    std::string output = captured_output.str();

    assert(output.find("Event A has been logged") != std::string::npos);
    assert(output.find("Event B has been logged") != std::string::npos);
    assert(output.find("Event C has been logged") != std::string::npos);
}