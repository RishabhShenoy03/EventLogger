#include <chrono>
#include <iostream>
#include <format>

#include "logger.hpp"

Logger::Logger(std::ostream& output): output_(output) {}

std::string Logger::timeToString(const std::chrono::system_clock::time_point& timestamp) {
    return std::format("{:%d-%m-%Y %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(timestamp));
}

std::string Logger::eventtypeToString(const EventType& type) {
    switch (type) {
        case EventType::DeviceOffline:      return "Device Offline";
        case EventType::DeviceOnline:       return "Device Online";
        case EventType::MotionDetected:     return "Motion Detected";
        case EventType::SensorReading:      return "Sensor Reading";
        default:                            return "Unknown";
    }
}

void Logger::print_event(const Event& event) {
    std::string formatted_event = timeToString(event.timestamp)     + " | "
                                + event.source                      + " | "
                                + eventtypeToString(event.type)     + " | "
                                + event.payload                     + '\n';
    output_ << formatted_event;
}

void Logger::log(const Event& event) {
    print_event(event);
}