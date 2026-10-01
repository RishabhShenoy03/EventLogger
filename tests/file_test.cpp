#include <fstream>
#include <iostream>
#include <cassert>
#include <array>
#include <thread>

#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"

int main() {

    // FIFO order verified
    auto fifo_test = [](){
        EventQueue queue{};

        Event first{
            .type = EventType::MotionDetected,
            .source = "filetest",
            .timestamp = std::chrono::system_clock::time_point{std::chrono::seconds{10}},
            .payload = "First"
        };
        Event second{
            .type = EventType::MotionDetected,
            .source = "filetest",
            .timestamp = std::chrono::system_clock::time_point{std::chrono::seconds{20}},
            .payload = "Second"
        };
        
        {
        std::jthread({
            queue.push(first);
            queue.push(second);
        });

        std::jthread({
            
        })


        }
        
        auto first_pop = queue.try_pop();
        auto second_pop = queue.try_pop();

        assert(first_pop.has_value());
        assert(second_pop.has_value());
        assert(first_pop->payload == "First");
        assert(second_pop->payload == "Second");

        assert(!queue.try_pop()); // return std::nullopt
    };

    // empty and closed queue returns nullopt
    auto empty_queue_test = [](){ 
        EventQueue queue{};
        queue.close();
        assert(!queue.wait_and_pop());
    };

    // closed queue still processes pending events
    auto pending_test = [](){
        EventQueue queue{};

        Event first{
            .type = EventType::MotionDetected,
            .source = "filetest",
            .timestamp = std::chrono::system_clock::time_point{std::chrono::seconds{10}},
            .payload = "First"
        };
        Event second{
            .type = EventType::MotionDetected,
            .source = "filetest",
            .timestamp = std::chrono::system_clock::time_point{std::chrono::seconds{20}},
            .payload = "Second"
        };
        queue.push(first);
        queue.push(second);
        auto first_pop = queue.wait_and_pop();
        auto second_pop = queue.wait_and_pop();
        auto third_pop = queue.wait_and_pop();

        assert(first_pop.has_value());
        assert(second_pop.has_value());
        assert(first_pop->payload == "First");
        assert(second_pop->payload == "Second");

        assert(!third_pop);
    };

    // pushing wakes a waiting consumer


    





}