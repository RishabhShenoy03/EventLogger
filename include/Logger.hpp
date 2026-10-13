#pragma once

#include <ostream>
#include <string>
#include <mutex>
#include <tgbot/tgbot.h>

#include "Event.hpp"

class Logger{

private:
    std::ostream& output_;
    std::string latest_event_ = "";
    std::size_t event_count_ = 0;
    std::mutex mutex_;
public:
    Logger(std::ostream& output);
    [[nodiscard]] std::string eventtypeToString(const EventType& type);
    [[nodiscard]] std::string timeToString(const std::chrono::system_clock::time_point& timestamp);
    [[nodiscard]] std::string format_event(const Event& event);
    void log_event(const std::string& formatted_string);
    [[nodiscard]] std::string get_latest();
    [[maybe_unused]] void set_latest(const std::string& latest);
    [[nodiscard]] std::size_t get_count();
};