#pragma once

enum class PersonID {
    Rishabh, Mummy, Daddy, Megan, Shreya, Unknown
};

struct PersonMatch {
    PersonID id;
    double similarity;
};