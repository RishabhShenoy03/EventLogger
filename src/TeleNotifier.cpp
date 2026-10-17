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

#include "TeleNotifier.hpp"
#include "Event.hpp"
#include "Logger.hpp"

using json = nlohmann::json;

TeleNotifier::TeleNotifier(const char* token, Logger& log, std::int64_t admin_id)
    : bot_(TgBot::Bot(token)), logs_(log), admin_id_(admin_id)
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
        {
            std::scoped_lock lock(mutex_);
            std::string payload = "Latest event: \n" + logs_.get_latest();
            bot_.getApi().sendMessage(message->chat->id, payload);
        }
    });

    bot_.getEvents().onAnyMessage([this](std::shared_ptr<TgBot::Message> message) {
        insert_user(message);
    });

    bot_.getEvents().onCommand("admin_sublist",[this](std::shared_ptr<TgBot::Message> message) {
        if (message->chat->id != admin_id_) {
            bot_.getApi().sendMessage(message->chat->id, "Bro thought he was an admin");
            return;
        }      
        bot_.getApi().sendMessage(admin_id_, subs_to_string());
    });

    bot_.getEvents().onCommand("admin_subcount", [this](std::shared_ptr<TgBot::Message> message) {
        std::scoped_lock lock(mutex_);
        if (message->chat->id != admin_id_) {
            bot_.getApi().sendMessage(message->chat->id, "Bro thought he was an admin");
            return;
        }
        
        std::string payload = "Subscribers: " + std::to_string(subscribers_.size());
        bot_.getApi().sendMessage(admin_id_, payload);
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
    
    for (auto subscriber : recipients) {
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
        if (it != subscribers_.end()) { // not in sub list
            subscribers_.erase(it);
            removed = true;
        }        
    }

    if (!removed) {
        bot_.getApi().sendMessage(
            pending_chat_id,
            "You weren't subbed yet, but registration has been cancelled");
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
