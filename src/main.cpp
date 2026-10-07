#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"
#include "CameraCapture.hpp"
#include "FaceProcessor.hpp"
#include "FaceRegistry.hpp"

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

int main() {
    CameraCapture camera{};
    FaceProcessor processor{};
    FaceRegistry fellas{};

    EventQueue queue{};
    Logger logs{std::cout};
    FakeSensor fake_sensor{};

    std::jthread producer([&camera, &processor] {
        while (true) {
            cv::Mat frame = camera.capture();
            cv::Mat faces = processor.detect(frame);
            std::vector<cv::Mat> persons_spotted = processor.extract(frame, faces);
            // something to recognise
        }
    });

    std::jthread producer2([&fake_sensor, &queue] {
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