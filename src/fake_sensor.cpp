#include "fake_sensor.hpp"

FakeSensor::FakeSensor() {

    Event event_a{EventType::MotionDetected, "FakeSensor", std::chrono::system_clock::time_point{
        std::chrono::seconds{10}}, "Event A"};
    Event event_b{EventType::SensorReading, "FakeSensor", std::chrono::system_clock::time_point{
        std::chrono::seconds{20}}, "Event B"};
    Event event_c{EventType::DeviceOnline, "FakeSensor", std::chrono::system_clock::time_point{
        std::chrono::seconds{30}}, "Event C"};
    Event event_d{EventType::DeviceOnline, "FakeSensor ", std::chrono::system_clock::time_point{
        std::chrono::seconds{40}}, "Event D"};

    events_.push_back(event_a);
    events_.push_back(event_b);
    events_.push_back(event_c);
    events_.push_back(event_d);
}

std::optional<Event> FakeSensor::sense_fake_event(){
    if (index_ < events_.size()){
        return events_[index_++]; // return events_[index_], then post-increment index_
    }
    return std::nullopt;
}