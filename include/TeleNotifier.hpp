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
    const std::int64_t admin_id_;
    void load_subscribers();
    [[nodiscard]] bool isSubbed(std::int64_t id_to_check);
    [[nodiscard]] std::vector<Subscriber>::iterator find_sub(std::int64_t id_to_find);
    void insert_user(const std::shared_ptr<TgBot::Message>& message);
    void erase_user(const std::shared_ptr<TgBot::Message>& message);
public:
    TeleNotifier(const char* token, Logger& logs, std::int64_t admin_id);
    void long_poll(std::stop_token stop);
    void send_notif(const std::string& formatted_event);
    void save_subscribers();
    [[nodiscard]] std::string subs_to_string();
};