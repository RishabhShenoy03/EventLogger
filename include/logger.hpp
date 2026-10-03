#pragma once

#include <ostream>

#include "event.hpp"

class Logger{

private:
    void print_event(const Event& event);
    std::ostream& output_;

public:
    Logger(std::ostream& output);
    std::string eventtypeToString(const EventType& type);
    std::string timeToString(const std::chrono::system_clock::time_point& timestamp);
    void log(const Event& event);
};