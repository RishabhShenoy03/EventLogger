#include <iostream>
#include <vector>
#include <optional>
#include <unordered_map>

#include "person_types.hpp"
#include "PresenceTracker.hpp"

using Clock = std::chrono::steady_clock;
using Sysclock = std::chrono::system_clock;

std::vector<Event> PresenceTracker::update(const std::vector<PersonMatch>& matches, const Clock::time_point& steady_now, const Sysclock::time_point& sys_now) {
    std::vector<Event> changes = erase_stale_entries(steady_now, sys_now);
    std::vector<Event> entries = handle_matches(steady_now, sys_now, matches);
    changes.insert(changes.end(), entries.begin(), entries.end());
    return changes;
}

std::vector<Event> PresenceTracker::erase_stale_entries(Clock::time_point steady_now, Sysclock::time_point sys_now) {
    std::vector<Event> leaving;
    for (auto it = inside_room_.begin(); it != inside_room_.end(); ) {
        if (steady_now - it->second > time_absence) {
            leaving.push_back(Event{
                EventType::PersonLeave,
                "Producer Thread",
                sys_now,
                person_leave_payload(it->first)
            });
            it = inside_room_.erase(it);
        }
        else {
            it++;
        }
    }
    return leaving;
}

std::vector<Event> PresenceTracker::handle_matches(const Clock::time_point& steady_now, const Sysclock::time_point& sys_now, const std::vector<PersonMatch>& matches) {
    std::vector<Event> match_events;
    bool saw_unknown = false;
    for (const auto& match : matches) {
        if (match.id != PersonID::Unknown) {    // known person detected
            auto it = inside_room_.find(match.id);
            if (it != inside_room_.end()) { // if map contains the same person already, update last seen
                it->second = steady_now;
            }
            else if (!last_entry_logged_ || steady_now - *last_entry_logged_ >= time_debounce) { // if not inside, add to map, push event, set last_entry_logged
                match_events.push_back(Event{
                    EventType::PersonEnter,
                    "Producer Thread",
                    sys_now,
                    person_enter_payload(match)
                });
                inside_room_[match.id] = steady_now; // only insert when not within the debounce time limit
                last_entry_logged_ = steady_now;
            }
        }
        else { // unknown person detected
            saw_unknown = true;
            if (inside_room_.find(match.id) != inside_room_.end()) { // unknown already inside map
                inside_room_[PersonID::Unknown] = steady_now;
                continue;   
            }

            // first time unknown spotted, set unknown spot time to check against in 3 seconds
            if (!pending_unknown_since_.has_value()) {
                pending_unknown_since_ = steady_now;
                continue;
            }

            // more than 3s since unknown detected, still unknown being detected
            // Also outside of debounce limit
            if (steady_now - *pending_unknown_since_ >= unknown_limit
                && (!last_entry_logged_ || steady_now - *last_entry_logged_ >= time_debounce)) {
                match_events.push_back(Event{
                    EventType::PersonEnter,
                    "Producer Thread",
                    sys_now,
                    person_enter_payload(match)
                });
                inside_room_[PersonID::Unknown] = steady_now; // update last seen
                pending_unknown_since_.reset();
                last_entry_logged_ = steady_now;
            }
        }
    }
    if (!saw_unknown) {
        pending_unknown_since_.reset();
    }
    return match_events;
}