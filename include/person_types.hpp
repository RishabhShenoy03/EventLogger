#pragma once

#include <string>

enum class PersonID {
    Rishabh, Mummy, Daddy, Megan, Shreya, Unknown
};

struct PersonMatch {
    PersonID id;
    double similarity;
};

[[nodiscard]] PersonID stringToPersonID(const std::string& person_name);
[[nodiscard]] std::string personIDToString(const PersonID& personid);
[[nodiscard]] std::string person_enter_payload(const PersonMatch& match);
[[nodiscard]] std::string person_leave_payload(const PersonID& personid);