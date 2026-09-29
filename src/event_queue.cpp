#include <utility>

#include "event_queue.hpp"

[[nodiscard]] std::size_t EventQueue::size() const {
    // std::lock_guard<std::mutex> lock(mutex_);
    return events_.size();
}

[[nodiscard]] bool EventQueue::empty() const {
    return events_.size() == 0;
}

void EventQueue::clear() {
    events_.clear();      
}

[[nodiscard]] std::optional<Event> EventQueue::try_pop() {
    // std::lock_guard<std::mutex> lock(mutex_);
    if (events_.empty()) {
        return std::nullopt;
    }

    Event event = std::move(events_.front());
    events_.pop_front();
    return event;
}

void EventQueue::push(const Event& event){
    events_.push_back(event);
}