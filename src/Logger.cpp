#include <chrono>
#include <iostream>
#include <format>
#include <tgbot/tgbot.h>
#include "Logger.hpp"

Logger::Logger(std::ostream& output): output_(output) {}

[[nodiscard]] std::string Logger::timeToString(const std::chrono::system_clock::time_point& timestamp) {
    auto sgt = timestamp + std::chrono::hours{8};
    return std::format("{:%d-%m-%Y %H:%M:%S} SGT", sgt);
}

[[nodiscard]] std::string Logger::eventtypeToString(const EventType& type) {
    switch (type) {
        case EventType::PersonEnter :    return "Person Enter";
        case EventType::PersonLeave :    return "Person Left";
        default                     :    return "Unknown";
    }
}

[[nodiscard]] std::string Logger::format_event(const Event& event) {
    std::string formatted_event = timeToString(event.timestamp)     + " | "
                                + event.source                      + " | "
                                + eventtypeToString(event.type)     + " | "
                                + event.payload                     + '\n';
    return formatted_event;
}

void Logger::log_event(const std::string& formatted_string) {
    output_ << (formatted_string);
    latest_event_ = formatted_string;
}