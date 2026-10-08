#pragma once

enum class PersonID {
    Rishabh, Mummy, Daddy, Megan, Shreya,
    COUNT
};

struct PersonMatch {
    PersonID id;
    double similarity;
};