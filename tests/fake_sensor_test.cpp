#include <cassert>

#include "fake_sensor.hpp"

int main() {
    FakeSensor fs{};
    int i = 0;

    while (i < 4){
        auto event_check = fs.sense_fake_event();
        assert(event_check.has_value());
        assert(event_check->source == "FakeSensor");
        i++;
    }

    auto event_check_null = fs.sense_fake_event();
    assert(!event_check_null.has_value());
}