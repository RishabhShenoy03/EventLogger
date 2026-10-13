#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <csignal>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <cstdlib>
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

    std::ofstream logfile{"outputs/events.log"};

    const char* token = std::getenv("TGBOT_API_KEY");
    if (token == nullptr || *token == '\0') {
        std::cerr << "TGBOT_BOT_TOKEN is not set\n";
        return 1;
    }

    EventQueue queue{};
    Logger logs{logfile};
    TeleNotifier telenotif {token};

    std::signal(SIGINT, request_shutdown);
    std::signal(SIGTERM, request_shutdown);

    std::jthread producer([&queue, &camera, &registry, &processor] (std::stop_token stop)
    {
        std::unordered_map<PersonID, Clock::time_point> inside_room;
        std::optional<Clock::time_point> pending_unknown_since;
        std::optional<Clock::time_point> last_entry_logged;

        const auto time_absence = std::chrono::seconds{30};
        const auto time_debounce = std::chrono::seconds{12};
        const auto unknown_limit = std::chrono::seconds{3};

        while (!stop.stop_requested()) {
            cv::Mat frame = camera.capture();
            cv::Mat faces = processor.detect(frame);
            std::vector<cv::Mat> persons_spotted = processor.extract(frame, faces);

            auto now = Clock::now();
            
            // Erase stale entries (>30 seconds)
            for (auto it = inside_room.begin(); it != inside_room.end(); ) {
                if (now - it->second > time_absence) {
                    queue.push(Event{
                        EventType::PersonLeave,
                        "Producer Thread",
                        Sysclock::now(),
                        registry.person_leave_payload(it->first)
                    });
                    it = inside_room.erase(it);
                }
                else {
                    it++;
                }
            }

            bool saw_unknown = false;

            for (const auto& person : persons_spotted) {
                PersonMatch match = registry.match(person);
                
                if (match.id != PersonID::Unknown) {    // known person detected
                    auto it = inside_room.find(match.id);
                    if (it != inside_room.end()) { // if map contains the same person already, update last seen
                        it->second = now;
                    }
                    else if (!last_entry_logged || now - *last_entry_logged >= time_debounce) { // if not inside, add to map, push event, set last_entry_logged
                        queue.push(Event{
                            EventType::PersonEnter,
                            "Producer Thread",
                            Sysclock::now(),
                            registry.person_enter_payload(match)
                        });
                        inside_room[match.id] = now; // only insert when not within the debounce time limit
                        last_entry_logged = now;
                    }
                }
                else { // unknown person detected
                    saw_unknown = true;
                    if (inside_room.find(match.id) != inside_room.end()) { // unknown already inside map
                        inside_room[PersonID::Unknown] = now;
                        continue;   
                    }

                    // first time unknown spotted, set unknown spot time to check against in 3 seconds
                    if (!pending_unknown_since.has_value()) {
                        pending_unknown_since = now;
                        continue;
                    }

                    // more than 3s since unknown detected, still unknown being detected
                    // Also outside of debounce limit
                    if (now - *pending_unknown_since >= unknown_limit
                        && (!last_entry_logged || now - *last_entry_logged >= time_debounce)) {
                        queue.push(Event{
                            EventType::PersonEnter,
                            "Producer Thread",
                            Sysclock::now(),
                            registry.person_enter_payload(match)
                        });
                        inside_room[PersonID::Unknown] = now; // update last seen
                        pending_unknown_since.reset();
                        last_entry_logged = now;
                    }
                }
            }
            if (!saw_unknown) {
                pending_unknown_since.reset();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
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

    std::jthread telebot([&telenotif] {
        telenotif.long_poll();
    });

    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    producer.request_stop();
    producer.join();
   
    return 0;
}