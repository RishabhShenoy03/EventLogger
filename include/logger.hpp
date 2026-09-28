#pragma once

#include <ostream>

#include "event.hpp"

class Logger{

private:
    void print_event(const Event& event);
    std::ostream& output_;

public:

    Logger(std::ostream& output);
    void log(const Event& event);
};