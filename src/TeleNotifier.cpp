#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread> 
#include <mutex>
#include <tgbot/tgbot.h>
#include <algorithm>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include "TeleNotifier.hpp"
#include "Event.hpp"
#include "Logger.hpp"

using json = nlohmann::json;

TeleNotifier::TeleNotifier(const char* token, Logger& log, std::int64_t admin_id)
    : bot_(TgBot::Bot(token)), logs_(log), admin_id_(admin_id),
    quick_tips_(
        "Commands:\n"
        "/help - Show available commands\n"
        "/start - Subscribe or update your name\n"
        "/stop - Unsubscribe or cancel registration\n"
        "/latest - View the latest event\n"
        "/contact <message> - Message the admin\n"
        "\nAdmin only:\n"
        "/subs - List subscribers and chat IDs\n"
        "/count - Show subscriber count\n"
        "/msg <chat_id> <message> - Message a subscriber\n"
        "/broadcast <message> - Message all subscribers except admin\n"
    )
{
    load_subscribers();
}

void TeleNotifier::long_poll(std::stop_token stop) {
    bot_.getEvents().onCommand("start", [this](std::shared_ptr<TgBot::Message> message) {
        bot_.getApi().sendMessage(message->chat->id, "Hello! Please choose/update your username:D");
        {
            std::scoped_lock lock(mutex_);
            awaiting_name_.insert(message->chat->id);
        }
    });

    bot_.getEvents().onCommand("stop", [this](std::shared_ptr<TgBot::Message> message) {
        erase_user(message);
    });

    bot_.getEvents().onCommand("latest", [this](std::shared_ptr<TgBot::Message> message) {
        std::string payload = "Latest event: \n" + logs_.get_latest();
        bot_.getApi().sendMessage(message->chat->id, payload);
    });

    bot_.getEvents().onAnyMessage([this](std::shared_ptr<TgBot::Message> message) {
        insert_user(message);
    });

    bot_.getEvents().onCommand("subs",[this](std::shared_ptr<TgBot::Message> message) {
        if (message->chat->id != admin_id_) {
            bot_.getApi().sendMessage(message->chat->id, "Bro thought he was an admin");
            return;
        }      
        bot_.getApi().sendMessage(admin_id_, subs_to_string());
    });

    bot_.getEvents().onCommand("count", [this](std::shared_ptr<TgBot::Message> message) {
        if (message->chat->id != admin_id_) {
            bot_.getApi().sendMessage(message->chat->id, "Bro thought he was an admin");
            return;
        }
        
        std::size_t count{};
        {
            std::scoped_lock lock(mutex_);
            count = subscribers_.size();
        }
        std::string payload = "Subscribers: " + std::to_string(count);
        bot_.getApi().sendMessage(admin_id_, payload);
    });

    bot_.getEvents().onCommand("msg", [this](std::shared_ptr<TgBot::Message> message) {
        admin_pm(message);
    });

    bot_.getEvents().onCommand("contact", [this](std::shared_ptr<TgBot::Message> message) {
        pm_to_admin(message);
    });

    bot_.getEvents().onCommand("broadcast", [this](std::shared_ptr<TgBot::Message> message) {
        broadcast(message);
    });

    bot_.getEvents().onCommand("help", [this](std::shared_ptr<TgBot::Message> message) {
        send_help(message);
    }); 

    try {
        const auto me =   bot_.getApi().getMe();
        std::cout << "Bot Username: "
                  << me->username.value_or("") << "\n";
    } catch (const std::exception& e){
        std::cerr << "Getting bot username failed: "
                  << e.what() << "\n";
    }

    TgBot::TgLongPoll longPoll(bot_, 100, 2);

    while (!stop.stop_requested()) {
        try {
            longPoll.start();
        } catch (const std::exception& e) {
            std::cerr << "Telegram polling/command failed: "
                    << e.what() << '\n';

            // Wait up to 2 seconds before retrying.
            // Check for shutdown every 100 milliseconds.
            for (int i = 0; i < 20 && !stop.stop_requested(); ++i) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds{100}
                );
            }
        }
    }
}

void TeleNotifier::send_notif(const std::string& formatted_event) {    
    std::vector<Subscriber> recipients;
    {
        std::scoped_lock lock(mutex_);
        recipients = subscribers_;
    }
    
    for (const auto& subscriber : recipients) {
        try {
            bot_.getApi().sendMessage(subscriber.chat_id, formatted_event);
        } catch (const std::exception& e) {
            std::cout << "Notif failed for chat_id " << subscriber.chat_id
                        << ": " << e.what() << "\n";
        }
    }
}

void TeleNotifier::save_subscribers() {
    json data;
    data["subscribers"] = json::array();
    {
        std::scoped_lock lock(mutex_);
        for (const auto& sub : subscribers_) {
            data["subscribers"].push_back({
                {"chat_id", sub.chat_id},
                {"display_name", sub.display_name}
            });
        }
    }

    std::filesystem::create_directories("data");

    const std::filesystem::path temp{"data/subscribers.json.tmp"};
    const std::filesystem::path destination{"data/subscribers.json"};

    std::ofstream file{temp};
    if (!file) {
        throw std::runtime_error("Could not open subscriber file");
    }

    file << data.dump(4) << '\n';
    file.close();

    if (!file) {
        throw std::runtime_error("Could not write subscriber file");
    }
    std::filesystem::rename(temp, destination);
}

void TeleNotifier::load_subscribers() {
    const std::filesystem::path path{"data/subscribers.json"};

    if (!std::filesystem::exists(path)) {
        return;
    }

    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error("Could not load subscriber file");
    }

    json data;
    file >> data;

    const auto& records = data.at("subscribers");
    if (!records.is_array()) {
        throw std::runtime_error("Subscriber records must be an array");
    }

    std::vector<Subscriber> loaded;

    for (const auto& record : records) {
        loaded.push_back(Subscriber{
            record.at("chat_id").get<std::int64_t>(),
            record.at("display_name").get<std::string>()
        });
    }

    subscribers_ = std::move(loaded);
}

bool TeleNotifier::isSubbed(std::int64_t id_to_check) {
    auto it = subscribers_.begin();
    for (; it != subscribers_.end(); ++it) {
        if (it->chat_id == id_to_check) {
            return true;
        }
    }
    return false;
}

std::vector<TeleNotifier::Subscriber>::iterator TeleNotifier::find_sub(std::int64_t id_to_find) {
    auto it = subscribers_.begin();
    for (; it != subscribers_.end(); ++it) {
        if (it->chat_id == id_to_find) {
            break;
        }
    }
    return it;
}

void TeleNotifier::insert_user(const std::shared_ptr<TgBot::Message>& message) {
    const auto new_chat_id = message->chat->id;
    const std::string name = message->text.value_or("");
    // Ignore commands
    if (name.starts_with("/") ||
        name.find_first_not_of(" \t\r\n") == std::string::npos) {
        return;
    }
    
    bool updated = false;

    {
        std::scoped_lock lock(mutex_);

        // skips welcome message for non-name-update messages
        if (!awaiting_name_.contains(new_chat_id)) {
            return;
        }

        if (!isSubbed(new_chat_id)) {
            subscribers_.push_back(Subscriber{new_chat_id, name});
            updated = false;
        }
        else {
            auto it = find_sub(new_chat_id);
            it->display_name = name;
            updated = true;
        }
        awaiting_name_.erase(new_chat_id);
    }
    try {
        save_subscribers();
    } catch (const std::exception& e) {
        std::cerr << "Could not save subscribers: " << e.what() << '\n';
        bot_.getApi().sendMessage(
            new_chat_id,
            "Your name is set for this session, but saving failed."
        );
    }

    const std::string reply = updated
        ? "Name Updated: Welcome back, " + name
        : "New User: Welcome, " + name;
    bot_.getApi().sendMessage(new_chat_id, reply);
    send_help(message);

    const std::string admin_notif = updated
        ? "Name updated: " + name + ", " + std::to_string(new_chat_id)
        : "New User: " + name + ", " + std::to_string(new_chat_id);
    bot_.getApi().sendMessage(admin_id_, admin_notif);
}

std::string TeleNotifier::subs_to_string() {
    std::scoped_lock lock(mutex_);

    if (subscribers_.empty()) {
        return "No subscribers currently";
    }

    std::string message = "Subscriber List:\n";
    for (const auto& s : subscribers_) {
        message += s.display_name + ", "
                + std::to_string(s.chat_id) + "\n";
    }
    return message;
}

void TeleNotifier::erase_user(const std::shared_ptr<TgBot::Message>& message) {
    const auto pending_chat_id = message->chat->id;
    bool removed = false;
    {
        std::scoped_lock lock(mutex_);
        awaiting_name_.erase(pending_chat_id);

        auto it = find_sub(pending_chat_id);
        if (it != subscribers_.end()) { // found in sub list
            subscribers_.erase(it);
            removed = true;
        }        
    }

    if (!removed) {
        bot_.getApi().sendMessage(
            pending_chat_id,
            "You weren't subbed yet, but registration has been cancelled"
        );
        return;
    }

    try {
        save_subscribers();
    } catch (const std::exception& e) {
        std::cerr << "Could not save subscribers: "
                    << e.what() << "\n";
        bot_.getApi().sendMessage(
            pending_chat_id,
            "Unsubscribed for this session, but saving failed."
        );
        return;
    }
    bot_.getApi().sendMessage(pending_chat_id, "You have unsubbed from RishberryPi");
}

void TeleNotifier::admin_pm(const std::shared_ptr<TgBot::Message>& message) {
    if (message->chat->id != admin_id_) {
        bot_.getApi().sendMessage(message->chat->id, "Bro thought he was an admin");
        return;
    }

    char delimiter = ' ';
    std::istringstream input{message->text.value_or("")};

    std::string command;
    std::string recipient_id_text;
    std::string payload;

    std::getline(input, command, delimiter);
    std::getline(input, recipient_id_text, delimiter);
    std::int64_t recipient_id_int{};
    try {
        recipient_id_int = std::stoll(recipient_id_text);
    } catch (const std::exception& e) {
        std::cerr << "Conversion to int64 failed" << e.what() << "\n";
        return;
    }
    std::getline(input, payload, '\0');
    if (isSubbed(recipient_id_int)) {
        bot_.getApi().sendMessage(
            recipient_id_int,
            "Incoming message from the admin:\n\n" + payload
        );
        bot_.getApi().sendMessage(admin_id_, "Message successfully sent");
    }
    else {
        bot_.getApi().sendMessage(admin_id_, "Person not in sub list");
    }
}

void TeleNotifier::pm_to_admin(const std::shared_ptr<TgBot::Message>& message) {
    std::string sender_name;
    const std::int64_t sender_id = message->chat->id;

    {
        std::scoped_lock lock(mutex_);
        auto it = find_sub(sender_id);
        if (it != subscribers_.end()) {
            sender_name = it->display_name;
        }
        else {
            sender_name = "Unregistered User";
        }
    }

    char delimiter = ' ';

    std::istringstream input{message->text.value_or("")};

    std::string command;
    std::string message_body;   // actual message
    std::string payload;        // with sender info for admin to read

    std::getline(input, command, delimiter);
    std::getline(input, message_body, '\0');

    payload = "Incoming message from " + sender_name + " ("
            + std::to_string(sender_id)+ "):\n\n" + message_body;

    try {
        bot_.getApi().sendMessage(admin_id_, payload);
    } catch (const std::exception& e) {
        std::cerr << "Failed to send to admin"
                  << ": " << e.what() << "\n";
        return;
    }
    
    try {
        bot_.getApi().sendMessage(sender_id, "Message successfully sent");
    } catch (const std::exception& e) {
        std::cerr << "Failed to notify sender"
                  << ": " << e.what() << "\n";
    }
}

void TeleNotifier::broadcast(const std::shared_ptr<TgBot::Message>& message) {
    if (message->chat->id != admin_id_) {
        bot_.getApi().sendMessage(message->chat->id, "Bro thought he was an admin");
        return;
    }

    std::vector<Subscriber> recipients;
    {
        std::scoped_lock lock(mutex_);
        recipients = subscribers_;
    }

    char delimiter = ' ';
    std::istringstream input{message->text.value_or("")};

    std::string command;
    std::string message_body;
    std::string payload;

    std::getline(input, command, delimiter);
    std::getline(input, message_body);

    payload = "Incoming broadcast from admin:\n\n"
            + message_body;

    bool sent_all = true;
    
    for (const auto& subscriber : recipients) {
        if (subscriber.chat_id == admin_id_) {
            continue;
        }

        try {
            bot_.getApi().sendMessage(subscriber.chat_id, payload);
        } catch (const std::exception& e) {
            std::cerr << "Notif failed for chat_id " << subscriber.chat_id
                        << ": " << e.what() << "\n";
            sent_all = false;
        }
    }

    if (sent_all) {
        bot_.getApi().sendMessage(admin_id_, "Broadcast succeeded!");
    } else {
        bot_.getApi().sendMessage(admin_id_, "Some messages failed to send");
    }
}

void TeleNotifier::send_help(const std::shared_ptr<TgBot::Message>& message) {
    try {
        bot_.getApi().sendMessage(message->chat->id, quick_tips_);
    } catch (const std::exception& e) {
        std::cerr << "Failed to send help"
                  << ": " << e.what() << "\n";
    }
}