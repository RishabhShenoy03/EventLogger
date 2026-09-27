#pragma once
#include <chrono>
#include <string>

enum class EventType {
    MotionDetected,
    SensorReading,
    DeviceOnline,
    DeviceOffline
};

struct Event {
    EventType type;
    std::string source;
    std::chrono::system_clock::time_point timestamp;
    std::string payload;
};