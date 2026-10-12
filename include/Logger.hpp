#pragma once

#include <ostream>
#include <string>
#include <tgbot/tgbot.h>

#include "Event.hpp"

class Logger{

private:
    void send_event(const Event& event);
    std::ostream& output_;
    const std::string token_;
    const std::int64_t chat_id_ = 697267727;
    TgBot::Bot bot;

public:
    Logger(std::ostream& output, const std::string& token);
    [[nodiscard]] std::string eventtypeToString(const EventType& type);
    [[nodiscard]] std::string timeToString(const std::chrono::system_clock::time_point& timestamp);
    void log(const Event& event);
};