#include <cassert>
#include <chrono>
#include <sstream>

#include "logger.hpp"

int main() {
    std::ostringstream captured_output;
    Logger logs{captured_output};

    Event A{EventType::MotionDetected, "LoggerTest", std::chrono::system_clock::time_point{
        std::chrono::seconds{1}}, "Event A"};
    Event B(EventType::SensorReading, "LoggerTest", std::chrono::system_clock::time_point{
        std::chrono::seconds{2}}, "Event B");
    Event C(EventType::DeviceOnline, "LoggerTest", std::chrono::system_clock::time_point{
        std::chrono::seconds{3}}, "Event C");
    
    logs.log(A);
    logs.log(B);
    logs.log(C);

    std::string output = captured_output.str();

    assert(output.find("Event A has been logged") != std::string::npos);
    assert(output.find("Event B has been logged") != std::string::npos);
    assert(output.find("Event C has been logged") != std::string::npos);
}