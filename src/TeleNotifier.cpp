#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tgbot/tgbot.h>
#include <algorithm>

#include "TeleNotifier.hpp"
#include "Event.hpp"
#include "Logger.hpp"

TeleNotifier::TeleNotifier(const char* token, Logger& logs)
    : bot_(TgBot::Bot(token)), logs_(logs) {}

void TeleNotifier::long_poll() {
    bot_.getEvents().onCommand("start", [this](std::shared_ptr<TgBot::Message> message) {
        bot_.getApi().sendMessage(message->chat->id, "Hi!");
        std::scoped_lock lock(mutex_);
        if (std::ranges::find(chat_id_list_, message->chat->id) == chat_id_list_.end()) {
            chat_id_list_.push_back(message->chat->id);
        }
    });

    bot_.getEvents().onCommand("status", [this](std::shared_ptr<TgBot::Message> message) {
        std::scoped_lock lock(mutex_);
        bot_.getApi().sendMessage(message->chat->id, logs_.get_latest());
    });

    try {
        std::cout << "Bot username: " << bot_.getApi().getMe()->username.value_or("") << std::endl;
        TgBot::TgLongPoll longPoll(bot_);
        longPoll.startLoop();
    } catch (const TgBot::TgException& e) {
        std::cout << "error: " << e.what() << std::endl;
    }
}

void TeleNotifier::send_notif(const std::string& formatted_event) {
    std::scoped_lock lock(mutex_);
    for (auto chat_id : chat_id_list_) {
        bot_.getApi().sendMessage(chat_id, formatted_event);
    }
}