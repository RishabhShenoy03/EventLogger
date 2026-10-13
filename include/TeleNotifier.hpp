#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <stop_token>
#include <tgbot/tgbot.h>

#include "Event.hpp"
#include "Logger.hpp"

class TeleNotifier {
    std::vector<std::int64_t> chat_id_list_;
    TgBot::Bot bot_;
    std::mutex mutex_;
    Logger& logs_;

public:
    TeleNotifier(const char* token, Logger& logs);
    void long_poll(std::stop_token stop);
    void send_notif(const std::string& formatted_event);
};