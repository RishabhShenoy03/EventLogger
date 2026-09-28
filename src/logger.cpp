#include <chrono>
#include <iostream>
#include <format>

#include "logger.hpp"

Logger::Logger(std::ostream& output): output_(output){}

void Logger::print_event(const Event& event){
    std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", event.timestamp);
    this->output_ << timestamp << std::endl;
    this->output_ << event.payload << " has been logged" << std::endl;
}

void Logger::log(const Event& event){
    print_event(event);
}