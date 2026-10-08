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
        case EventType::PersonEnter :    return "Person Enter";
        case EventType::PersonLeave :    return "Person Left";
        default                     :    return "Unknown";
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