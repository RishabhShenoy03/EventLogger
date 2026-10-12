#include <chrono>
#include <iostream>
#include <format>
#include <tgbot/tgbot.h>
#include "Logger.hpp"

Logger::Logger(std::ostream& output, const std::string& token)
: output_(output), token_(token), bot(TgBot::Bot(token_)) {}

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

void Logger::send_event(const Event& event) {
    std::string formatted_event = timeToString(event.timestamp)     + " | "
                                + event.source                      + " | "
                                + eventtypeToString(event.type)     + " | "
                                + event.payload                     + '\n';
    output_ << formatted_event;
    bot.getApi().sendMessage(chat_id_, formatted_event);
}

void Logger::log(const Event& event) {
    send_event(event);
}