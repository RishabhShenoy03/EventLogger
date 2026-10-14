#include <string>
#include "person_types.hpp"

PersonID stringToPersonID(const std::string& person_name) {
    if (person_name == "Rishabh") {
        return PersonID::Rishabh;
    }
    if (person_name == "Daddy") {
        return PersonID:: Daddy;
    }
    if (person_name == "Mummy") {
        return PersonID::Mummy;
    }
    if (person_name == "Megan") {
        return PersonID::Megan;
    }
    if (person_name == "Shreya") {
        return PersonID::Shreya;
    }
    return PersonID::Unknown;
}

std::string personIDToString(const PersonID& personid) {
    switch (personid) {
        case PersonID::Rishabh  : return "Rishabh";
        case PersonID::Daddy    : return "Daddy";
        case PersonID::Megan    : return "Megan";
        case PersonID::Mummy    : return "Mummy";
        case PersonID::Shreya   : return "Shreya";
        default                 : return "Unknown Person";
    }
}

std::string person_enter_payload(const PersonMatch& match) {
    if (match.id == PersonID::Unknown) {
        return "Unknown person has entered";
    }
    return personIDToString(match.id) + " has entered ("
        + std::to_string(match.similarity) + ")";
}

std::string person_leave_payload(const PersonID& personid) {
    return personIDToString(personid) + " has left";
}