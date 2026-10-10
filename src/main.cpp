#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <csignal>
#include <fstream>
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

    std::ofstream logfile{"outputs/events.log", std::ios::app};

    EventQueue queue{};
    Logger logs{logfile};

    std::signal(SIGINT, request_shutdown);
    std::signal(SIGTERM, request_shutdown);

    std::jthread producer([&queue, &camera, &registry, &processor] (std::stop_token stop)
    {

        std::unordered_map<PersonID, Clock::time_point> inside_room;
        const auto time_absence = std::chrono::seconds{8};

        while (!stop.stop_requested()) {
            cv::Mat frame = camera.capture();
            cv::Mat faces = processor.detect(frame);
            std::vector<cv::Mat> persons_spotted = processor.extract(frame, faces);

            auto now = Clock::now();
            
            // Erase stale entries (>8 seconds)
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
                if (inside_room.find(match.id) == inside_room.end()) {
                    queue.push(Event{
                        EventType::PersonEnter,
                        "Producer Thread",
                        Sysclock::now(),
                        registry.person_enter_payload(match)
                    });
                }
                inside_room[match.id] = Clock::now();
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