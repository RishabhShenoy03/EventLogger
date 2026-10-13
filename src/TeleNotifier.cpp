#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tgbot/tgbot.h>

#include "TeleNotifier.hpp"
#include "Event.hpp"

TeleNotifier::TeleNotifier() : bot(TgBot::Bot(token)) {
    bot.getEvents().onCommand("start", [&bot](std::shared_ptr<TgBot::Message> message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
        chat_id_list.push_back(message->chat->id);
    });
}

void TeleNotifier::send_notif(const Event& event) {
    for (auto chat_id : chat_id_list_) {
        bot.getApi().sendMessage(chat_id, Logger::format_event(event));
    }
}