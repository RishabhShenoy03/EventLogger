#include <cassert>
#include <sstream>
#include <array>
#include <thread>
#include <future>

#include "event_queue.hpp"
#include "logger.hpp"
#include "fake_sensor.hpp"

/*
    Check that when empty, queue 
*/
int main() {

    EventQueue queue{};
    std::promise<std::optional<Event>> promise;
    std::future<std::optional<Event>> future = promise.get_future();

    std::jthread logger_thread([&queue, &promise, &future]{
        auto check_event = queue.wait_and_pop(); // waits since queue is open and empty
        promise.set_value(check_event);
    });

    assert(future.wait_for(std::chrono::seconds(1)) == std::future_status::timeout); // no result yet as logger has not returned
    queue.close();
    assert(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready); // future is ready to return value
    assert(!future.get()); // returns std::nullopt
}