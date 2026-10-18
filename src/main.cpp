#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <csignal>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <cstdlib>
#include <stop_token>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

#include "EventQueue.hpp"
#include "Logger.hpp"
#include "CameraCapture.hpp"
#include "FaceProcessor.hpp"
#include "FaceRegistry.hpp"
#include "person_types.hpp"
#include "TeleNotifier.hpp"
#include "PresenceTracker.hpp"

using Clock = std::chrono::steady_clock;
using Sysclock = std::chrono::system_clock;

volatile std::sig_atomic_t shutdown_requested = 0;

void request_shutdown(int) {
    shutdown_requested = 1;
}

int main() {
    CameraCapture camera{};
    FaceProcessor processor{};
    FaceRegistry registry{processor};
    PresenceTracker presence{};

    std::filesystem::create_directories("outputs");

    std::ofstream logfile{"outputs/events.log", std::ios::app};
    if (!logfile) {
        std::cerr << "Could not open outputs/events.log\n";
        return 1;
    }

    const char* token = std::getenv("TGBOT_API_KEY");
    if (token == nullptr || *token == '\0') {
        std::cerr << "TGBOT_BOT_TOKEN is not set\n";
        return 1;
    }

    const char* admin_env = std::getenv("TGBOT_ADMIN_CHAT_ID");
    if (admin_env == nullptr || *admin_env == '\0') {
        std::cerr << "TGBOT_ADMIN_CHAT_ID is not set\n";
        return 1;
    }

    std::int64_t admin_id{};
    try {
        admin_id = std::stoll(admin_env);
    } catch (const std::exception& e) {
        std::cerr << "Conversion to int64 failed" << e.what() << "\n";
        return 1;
    }

    EventQueue queue{};
    Logger logs{logfile};
    TeleNotifier telenotif {token, logs, admin_id};

    std::signal(SIGINT, request_shutdown);
    std::signal(SIGTERM, request_shutdown);

    std::jthread producer([&queue, &camera, &registry, &processor, &presence]
                        (std::stop_token stop)
    {
        while (!stop.stop_requested()) {

            cv::Mat frame = camera.capture();
            cv::Mat faces = processor.detect(frame);
            std::vector<cv::Mat> persons_spotted = processor.extract(frame, faces);
            
            std::vector<PersonMatch> matches;
            matches.reserve(persons_spotted.size());

            auto steady_now = Clock::now();
            auto sys_now = Sysclock::now();

            for (const auto& person : persons_spotted) {
                matches.push_back(registry.match(person));
            }

            std::vector<Event> changes = presence.update(matches, steady_now, sys_now);

            for (const Event& event : changes) {
                queue.push(event);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        queue.close();
    });

    std::jthread consumer([&logs, &queue, &telenotif] {
        while (true) {
            auto opt_event = queue.wait_and_pop();
            if (!opt_event.has_value()) {
                break;
            }
            std::string formatted_event = logs.format_event(*opt_event);
            logs.log_event(formatted_event);
            telenotif.send_notif(formatted_event);
        }
    });

    std::jthread telebot([&telenotif](std::stop_token stop) {
        telenotif.long_poll(stop);
    });

    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    producer.request_stop();
    telebot.request_stop();

    producer.join();
    consumer.join();
    telebot.join();

    try {
        telenotif.save_subscribers();
    } catch (const std::exception& e){
        std::cerr << "Could not save subscribers: " << e.what() << '\n';
    }
   
    return 0;
}