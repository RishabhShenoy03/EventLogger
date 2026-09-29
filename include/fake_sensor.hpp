#include <vector>
#include <optional>

#include "event.hpp"

class FakeSensor{
private:
    std::vector<Event> events_;
    std::size_t index_ = 0;

public:
    FakeSensor();
    std::optional<Event> sense_fake_event();
};