#include <fstream>
#include <iostream>
#include <thread>

#include "event.hpp"
#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"

int main() {
    FakeSensor fake_sensor{};
    EventQueue queue{};
    std::ofstream logfile{"events.log", std::ios::app};
    if (!logfile){
        std::cerr << "Could not open events.log" << std::endl;
        return 1;
    }
    Logger logs{logfile};

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
    });

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

    
}