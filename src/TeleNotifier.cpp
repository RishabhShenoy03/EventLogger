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

TeleNotifier::TeleNotifier(const char* token, Logger& logs)
    : bot_(TgBot::Bot(token)), logs_(logs)
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

    bot_.getEvents().onAnyMessage([this](std::shared_ptr<TgBot::Message> message) {
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
            if (awaiting_name_.find(new_chat_id) != awaiting_name_.end()) {
                auto it = subscribers_.begin();
                
                // check if already subscribed, then update/add subscriber
                for (; it != subscribers_.end(); ++it) {
                    // already subscribed
                    if (it->chat_id == new_chat_id) {
                        it->display_name = name;
                        updated = true;
                        break;
                    }
                }
                if (it == subscribers_.end()) {
                    subscribers_.push_back(Subscriber{new_chat_id, name});
                    updated = false;
                }
                awaiting_name_.erase(new_chat_id);
            }
            else {
                return;
            }
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

bool TeleNotifier::isPresent(const std::int64_t& id_to_check) {
    auto it = subscribers_.begin();
    for (; it != subscribers_.end(); ++it) {
        if (it->chat_id == id_to_check) {
            return true;
        }
    }
    return false;
}

