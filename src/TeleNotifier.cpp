#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tgbot/tgbot.h>
#include <algorithm>
#include <unordered_set>

#include "TeleNotifier.hpp"
#include "Event.hpp"
#include "Logger.hpp"

TeleNotifier::TeleNotifier(const char* token, Logger& logs)
    : bot_(TgBot::Bot(token)), logs_(logs) {}

void TeleNotifier::long_poll(std::stop_token stop) {
    bot_.getEvents().onCommand("start", [this](std::shared_ptr<TgBot::Message> message) {
        bot_.getApi().sendMessage(message->chat->id, "Hello! Please choose a username:D");
        {
            std::scoped_lock lock(mutex_);
            awaiting_name_.insert(message->chat->id);
        }
    });

    bot_.getEvents().onAnyMessage([this](std::shared_ptr<TgBot::Message> message) {
        const auto new_chat_id = message->chat->id;
        const std::string name = message->text.value_or("");
        // Ignore commands
        if (name.starts_with("/") ||
            name.find_first_not_of(" \t\r\n") == std::string::npos) {
            return;
        }
        {
            std::scoped_lock lock(mutex_);
            if (awaiting_name_.find(new_chat_id) != awaiting_name_.end()) {
                auto it = subscribers_.begin();
                // check if already subscribed, then update/add subscriber
                for (; it != subscribers_.end(); ++it) {
                    // already subscribed
                    if (it->chat_id == new_chat_id) {
                        it->display_name = name;
                        break;
                    }
                }
                if (it == subscribers_.end()) {
                    subscribers_.push_back(Subscriber{new_chat_id, name});
                }

                awaiting_name_.erase(new_chat_id);
            }
        }
    });

    bot_.getEvents().onCommand("status", [this](std::shared_ptr<TgBot::Message> message) {
        {
            std::scoped_lock lock(mutex_);
            bot_.getApi().sendMessage(message->chat->id, logs_.get_latest());
        }
    });

    try {
        std::cout << "Bot username: " << bot_.getApi().getMe()->username.value_or("") << std::endl;
        TgBot::TgLongPoll longPoll(bot_, 100, 2);
        while (!stop.stop_requested()) {
            longPoll.start();
        }
    } catch (const TgBot::TgException& e) {
        std::cout << "error: " << e.what() << std::endl;
    }
}

void TeleNotifier::send_notif(const std::string& formatted_event) {
    {
        std::scoped_lock lock(mutex_);
        for (auto subscriber : subscribers_) {
            try {
                bot_.getApi().sendMessage(subscriber.chat_id, formatted_event);
            } catch (const TgBot::TgException& e) {
                std::cout << "error: " << e.what() << std::endl;
            }
        }
    }
}