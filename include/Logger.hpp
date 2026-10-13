#pragma once

#include <ostream>
#include <string>
#include <tgbot/tgbot.h>

#include "Event.hpp"

class Logger{

private:
    std::ostream& output_;
    std::string latest_event_;
public:
    Logger(std::ostream& output);
    [[nodiscard]] static std::string eventtypeToString(const EventType& type);
    [[nodiscard]] static std::string timeToString(const std::chrono::system_clock::time_point& timestamp);
    [[nodiscard]] static std::string format_event(const Event& event);
    void log_event(const std::string& formatted_string);
};