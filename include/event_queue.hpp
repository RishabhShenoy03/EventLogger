#pragma once

#include <deque>
#include <optional>
#include <cstddef>
#include <mutex>
#include <condition_variable>

#include "event.hpp"

class EventQueue {

private:
    std::deque<Event> events_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    bool closed_ = false;

public:
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;
    void clear();
    [[nodiscard]] std::optional<Event> try_pop();
    [[nodiscard]] std::optional<Event> wait_and_pop();
    void push(const Event& event);
    void close();
};