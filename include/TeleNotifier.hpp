#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tgbot/tgbot.h>

#include "Event.hpp"

class TeleNotifier {
    std::vector<std::int64_t> chat_id_list_;
    TgBot::Bot bot_;
    const std::string token_;

public:
    TeleNotifier() : bot(TgBot::Bot(token));
    void send_notif(const Event& event);
};