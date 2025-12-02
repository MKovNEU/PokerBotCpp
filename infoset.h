#pragma once

#include <cstdint>
#include "Enums/Moves.h"
#include <cstddef>
#include <vector>

class Card;

class Infoset {
private:
    size_t currentCardBit;
    size_t bettingHistoryBit;
    __uint128_t infoset;
    std::vector<Card> cards;
    std::vector<Moves> moves;

public:
    Infoset();
    void addCard(const Card &card);
    void addMove(Moves move);
    __uint128_t getInfoset() const;
    std::string toString() const;
};