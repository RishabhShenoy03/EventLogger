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
    struct Subscriber {
        std::int64_t chat_id;
        std::string display_name;
    };

    std::vector<Subscriber> subscribers_;
    TgBot::Bot bot_;
    std::mutex mutex_;
    Logger& logs_;
    std::unordered_set<std::int64_t> awaiting_name_;
    void load_subscribers();


public:
    TeleNotifier(const char* token, Logger& logs);
    void long_poll(std::stop_token stop);
    void send_notif(const std::string& formatted_event);
    void save_subscribers();

};