#include <cassert>
#include <sstream>
#include <array>

#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"

int main() {
    std::ostringstream output;
    Logger log{output};
    FakeSensor fs{};
    EventQueue queue{};

    // Check fake sensor events are accessed properly
    if (auto check_event = fs.sense_fake_event()) {
        queue.push(*check_event);
        assert(check_event->payload == "Event A");
    }


    if (auto check_event = fs.sense_fake_event()) {
        queue.push(*check_event);
        assert(check_event->payload == "Event B");
    }
    
    if (auto check_event = fs.sense_fake_event()) {
        queue.push(*check_event);
        assert(check_event->payload == "Event C");
    }
    
    if (auto check_event = fs.sense_fake_event()) {
        queue.push(*check_event);
        assert(check_event->payload == "Event D");
    }

    // Will be std::nullopt here as all 4 events have been seen
    assert(!fs.sense_fake_event());

    // Queue into Logger

    assert(queue.size() == 4);

    const std::array expected{ // FIFO expected payloads of FS events
        std::string{"Event A"},
        std::string{"Event B"},
        std::string{"Event C"},
        std::string{"Event D"}
    };

    // Check for each event in queue, payload matches expected payload in FIFO
    for (const auto& expected_payload : expected) {
        auto popped_event = queue.try_pop();

        assert(popped_event.has_value());
        assert(popped_event->payload == expected_payload);

        log.log(*popped_event);
    }

    assert(queue.empty());
    assert(!queue.try_pop()); // std::nullopt expected

    // Check logger

    std::string output_str = output.str();

    assert(output_str.find("Event A has been logged") != std::string::npos);
    assert(output_str.find("Event B has been logged") != std::string::npos);
    assert(output_str.find("Event C has been logged") != std::string::npos);
    assert(output_str.find("Event D has been logged") != std::string::npos);
}