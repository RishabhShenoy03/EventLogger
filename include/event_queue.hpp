#pragma once

#include <deque>
#include <optional>
#include <cstddef>

#include "event.hpp"

class EventQueue {

private:
    std::deque<Event> events_;

public:
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;
    void clear();
    [[nodiscard]] std::optional<Event> try_pop();
    void push(const Event& event);
};