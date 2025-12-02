#pragma once

#include <cstdint>
#include "Enums/Ranks.h"
#include "Enums/Suits.h"

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
};
