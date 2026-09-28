#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <cstddef>
#include <utility>

#include "event.hpp"


class EventQueue {

private:
    std::deque<Event> events_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool closed_ = false;

public:

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_.size();
    }

    std::optional<Event> try_pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (events_.empty()) {
            return std::nullopt;
        }

        Event event = std::move(events_.front());
        events_.pop_front();
        return event;
    }


        

};