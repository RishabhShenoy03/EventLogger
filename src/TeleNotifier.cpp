#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tgbot/tgbot.h>

#include "TeleNotifier.hpp"
#include "Event.hpp"

TeleNotifier::TeleNotifier() : bot_(TgBot::Bot(token_)) {
    bot_.getEvents().onCommand("start", [&bot_](std::shared_ptr<TgBot::Message> message) {
        bot_.getApi().sendMessage(message->chat->id, "Hi!");
        chat_id_list_.push_back(message->chat->id);
    });
}

void TeleNotifier::send_notif(const Event& event) {
    for (auto chat_id : chat_id_list_) {
        bot_.getApi().sendMessage(chat_id, Logger::format_event(event));
    }
}