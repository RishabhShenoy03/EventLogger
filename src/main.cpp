#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"
#include "CameraCapture.hpp"
#include "FaceProcessor.hpp"

#include <iostream>
#include <thread>

int main() {
    CameraCapture
    FakeSensor fake_sensor{};
    EventQueue queue{};
    Logger logs{std::cout};

    
    std::jthread producer([&fake_sensor, &queue]{
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
        }
    );

    std::jthread consumer([&logs, &queue]{
            while (true) {
                auto opt_event = queue.wait_and_pop();
                if (!opt_event.has_value()) {
                    break;
                }
                logs.log(*opt_event);
            }
        }
    );
    
    return 0;
}