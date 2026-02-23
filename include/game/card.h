#pragma once

#include <cstdint>
#include "include/enums/ranks.h"
#include "include/enums/suits.h"

struct Card {
    Ranks rank;
    Suits suit;

    Card(Ranks r, Suits s) : rank(r), suit(s) {}

    uint8_t encode() const {
        return rank * 4 + suit + 1;
    }

    std::string toString() const {
        return std::string(rankString(rank) + suitString(suit));
    }

    bool operator<(const Card &other) const {
        return this->encode() < other.encode();
    }

    bool operator==(const Card &other) const {
        return this->encode() == other.encode();
    }
};
