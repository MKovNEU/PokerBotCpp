#pragma once

#include <cstdint>
#include "include/enums/moves.h"
#include <cstddef>
#include <vector>

using ll  = long long int;

struct Card;

class Infoset {
private:
    size_t currentCardBit;
    size_t bettingHistoryBit;
    ll infoset;

public:
    Infoset();
    void addCard(const Card &card);
    void removeCard();
    void addBucket(int bucket);
    void addMove(Moves move);
    void popMove();
    ll getInfoset() const;
};