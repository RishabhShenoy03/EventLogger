#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <csignal>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

#include "event_queue.hpp"
#include "logger.hpp"
#include "CameraCapture.hpp"
#include "FaceProcessor.hpp"
#include "FaceRegistry.hpp"
#include "person_types.hpp"

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

    EventQueue queue{};
    Logger logs{logfile};

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

            if (persons_spotted.empty()) {
                pending_unknown_since.reset();
            }

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

            for (const auto& person : persons_spotted) {

                PersonMatch match = registry.match(person);

                if (match.id != PersonID::Unknown) { // known person detected
                    if (inside_room.find(match.id) == inside_room.end()
                        && now - *last_entry_logged >= time_debounce) { // not inside room yet
                        queue.push(Event{
                            EventType::PersonEnter,
                            "Producer Thread",
                            Sysclock::now(),
                            registry.person_enter_payload(match)
                        });
                        last_entry_logged = now;
                    }
                    inside_room[match.id] = now; // update time of last seen;
                }
                else { // unknown person detected
                    if (inside_room.find(match.id) != inside_room.end()) { // unknown already inside map
                        inside_room[PersonID::Unknown] = now;
                        continue;   
                    }

                    if (!pending_unknown_since.has_value()) {
                        pending_unknown_since = now;
                        continue;
                    }

                    if (now - *pending_unknown_since >= unknown_limit
                        && now - *last_entry_logged >= time_debounce) {  // more than 3s since unknown detected, still unknown
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
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        queue.close();
    });

    std::jthread consumer([&logs, &queue] {
        while (true) {
            auto opt_event = queue.wait_and_pop();
            if (!opt_event.has_value()) {
                break;
            }
            logs.log(*opt_event);
        }
    });

    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    producer.request_stop();
    producer.join();
   
    return 0;
}