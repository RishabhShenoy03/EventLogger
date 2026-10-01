#include <utility>
#include <thread>

#include "event_queue.hpp"

[[nodiscard]] std::size_t EventQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return events_.size();
}

[[nodiscard]] bool EventQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return events_.size() == 0;
}

void EventQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    events_.clear();      
}

[[nodiscard]] std::optional<Event> EventQueue::try_pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (events_.empty()) {
        return std::nullopt;
    }

    Event event = std::move(events_.front());
    events_.pop_front();
    return event;
}

[[nodiscard]] std::optional<Event> EventQueue::wait_and_pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    cond_.wait(lock, [this]{                        // wait (release mutex) until queue is not empty OR queue not closed
        return !events_.empty() || closed_;         // consumer wake when there is work OR when shutdown has happen
    });                                             // i.e. stop waiting if there is event to process OR no more events ever
    if (events_.empty() && closed_){
        return std::nullopt;
    }

    Event event = std::move(events_.front());
    events_.pop_front();
    return event;
}

void EventQueue::push(const Event& event){
    std::lock_guard<std::mutex> lock(mutex_);
    if (!closed_){
        events_.push_back(event);
    }   
    cond_.notify_one(); // wake a waiting consumer (logger)
}

void EventQueue::close() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        closed_ = true;
        lock.unlock();

    }
    cond_.notify_all(); // wake all waiting consumers (loggers)
}