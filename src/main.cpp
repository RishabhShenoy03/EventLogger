#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
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

int main() {
    CameraCapture camera{};
    FaceProcessor processor{};
    FaceRegistry registry{processor};

    EventQueue queue{};
    Logger logs{std::cout};

    std::jthread producer([&queue, &camera, &registry, &processor] {
        // int i = 0;
        while (true) {
            cv::Mat frame = camera.capture();
            cv::Mat faces = processor.detect(frame);
            std::vector<cv::Mat> persons_spotted = processor.extract(frame, faces);
            for (const auto& person : persons_spotted) {
                PersonMatch match = registry.match(person);
                
                queue.push(Event{
                    EventType::PersonEnter,
                    "Producer Thread",
                    std::chrono::system_clock::now(),
                    registry.id_to_payload(match)}
                );
            }
            // i++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        // queue.close();
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
    
    return 0;
}