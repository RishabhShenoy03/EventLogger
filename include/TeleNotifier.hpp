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

public:
    TeleNotifier(const char* token);
    void long_poll();
    void send_notif(const std::string& formatted_event);
};