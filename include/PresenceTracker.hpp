#pragma once

#include <vector>
#include <optional>
#include <unordered_map>
#include <chrono>

#include "Event.hpp"
#include "person_types.hpp"

using Clock = std::chrono::steady_clock;
using Sysclock = std::chrono::system_clock;

class PresenceTracker {
private:
    std::unordered_map<PersonID, Clock::time_point> inside_room_;
    std::optional<Clock::time_point> pending_unknown_since_;
    std::optional<Clock::time_point> last_entry_logged_;

    const std::chrono::seconds time_absence{600};
    const std::chrono::seconds time_debounce{30};
    const std::chrono::seconds unknown_limit{5};
    [[nodiscard]] std::vector<Event> erase_stale_entries(Clock::time_point steady_now, Sysclock::time_point sys_now);
    [[nodiscard]] std::vector<Event> handle_matches(const Clock::time_point& steady_now, const Sysclock::time_point& sys_now, const std::vector<PersonMatch>& matches);

public:
    PresenceTracker() = default;
    [[nodiscard]] std::vector<Event> update(const std::vector<PersonMatch>& matches, const Clock::time_point& steady_now, const Sysclock::time_point& sys_now);

};