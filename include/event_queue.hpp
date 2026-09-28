#pragma once

#include <deque>
#include <optional>
#include <cstddef>

#include "event.hpp"

class EventQueue {

private:
    std::deque<Event> events_;

public:
    std::size_t size() const;
    bool isEmpty() const;
    void clear();
    std::optional<Event> try_pop();
    void push(const Event& event);
};