#include <iostream>
#include <thread>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"
#include "CameraCapture.hpp"
#include "FaceProcessor.hpp"
#include "FaceRegistry.hpp"

int main() {
    CameraCapture camera{};
    FaceProcessor processor{};
    FaceRegistry registry{processor};

    EventQueue queue{};
    Logger logs{std::cout};
    FakeSensor fake_sensor{};

    std::jthread producer([&queue, &camera, &registry, &processor] {
        while (true) {
            std::optional<PersonMatch> id;

            cv::Mat frame = camera.capture();
            cv::Mat faces = processor.detect(frame);
            std::vector<cv::Mat> persons_spotted = processor.extract(frame, faces);
            for (const auto& person : persons_spotted) {
                id = registry.match(person);
                if (id.has_value()) {
                    queue.push(Event{
                        EventType::PersonEnter,
                        "Producer Thread",
                        std::chrono::system_clock::now(),
                        registry.id_to_payload(*id)}
                    );
                }
            }
        }
    });

    std::jthread producerfake([&fake_sensor, &queue] {
        while (true){
            auto opt_event = fake_sensor.sense_fake_event();
            if (opt_event.has_value()){
                queue.push(*opt_event);
            }
            else{
                queue.close();
                break;
            }
        }
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