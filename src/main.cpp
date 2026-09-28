#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"

#include <iostream>

int main() {
    FakeSensor fake_sensor{};
    EventQueue queue{};
    Logger logs{std::cout};

    while (true){
        auto opt_event = fake_sensor.sense_fake_event();
        if (opt_event.has_value()){
            queue.push(*opt_event);
        }
        else{
            break;
        }
    }
    
    while (true) {
        auto opt_event = queue.try_pop();
        if (!opt_event.has_value()) {
            break;
        }

        logs.log(*opt_event);
    }

    return 0;
}