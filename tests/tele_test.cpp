#include <iostream>
#include <memory>
#include <tgbot/tgbot.h>
#include <cstdlib>

int main() {
    const char* token = std::getenv("TGBOT_API_KEY");
    if (token == nullptr || *token == '\0') {
        std::cerr << "TGBOT_BOT_TOKEN is not set\n";
        return 1;
    }

    TgBot::Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](std::shared_ptr<TgBot::Message> message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });

    bot.getEvents().onAnyMessage([&bot](std::shared_ptr<TgBot::Message> message) {
        const auto text = message->text.value_or("");
        std::cout << "User wrote " << text << std::endl;
        std::cout << message->chat->id;
        if (text.starts_with("/start")) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + text);
    }); 
    try {
        std::cout << "Bot username: " << bot.getApi().getMe()->username.value_or("") << std::endl;
        TgBot::TgLongPoll longPoll(bot);
        longPoll.startLoop();
    } catch (const TgBot::TgException& e) {
        std::cout << "error: " << e.what() << std::endl;
    }
    return 0;
}