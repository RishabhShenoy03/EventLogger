#include <cassert>
#include <thread>
#include <future>
#include <optional>

#include "event_queue.hpp"

// closed queue still processes pending events
void pending_test() {
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
    queue.close();
    auto third_pop = queue.wait_and_pop();

    assert(first_pop.has_value());
    assert(second_pop.has_value());
    assert(first_pop->payload == "First");
    assert(second_pop->payload == "Second");

    assert(!third_pop);
}

// queue processes items in order of arrival
void fifo_test() {
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
    auto first_pop = queue.try_pop();
    auto second_pop = queue.try_pop();

    assert(first_pop.has_value());
    assert(second_pop.has_value());
    assert(first_pop->payload == "First");
    assert(second_pop->payload == "Second");

    assert(!queue.try_pop()); // return std::nullopt
}

// empty and closed queue returns nullopt
void empty_queue_test() { 
    EventQueue queue{};
    queue.close();
    assert(!queue.wait_and_pop());
};   

// push returns false if queue is closed
void push_after_close_test() {
    EventQueue queue{};
    queue.close();
    Event a{EventType::MotionDetected, "Push After Close Test", std::chrono::system_clock::time_point(std::chrono::seconds(1)), "Pushed"};
    assert(queue.push(a) == false);
}

// when queue is empty, wait_and_pop waits (release mutex), wake again when closed
void queue_shutdown_test() {
    EventQueue queue{};
    std::promise<std::optional<Event>> promise;
    std::future<std::optional<Event>> future = promise.get_future();

    std::jthread logger_thread([&queue, &promise]{
        auto check_event = queue.wait_and_pop(); // waits since queue is open and empty
        promise.set_value(check_event);
    });

    assert(future.wait_for(std::chrono::seconds(1)) == std::future_status::timeout); // no result yet as logger has not returned
    queue.close();
    assert(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready); // future is ready to return value
    assert(!future.get()); // returns std::nullopt
}

int main() {    
    fifo_test();
    empty_queue_test();
    pending_test();
    queue_shutdown_test();
    push_after_close_test();
}