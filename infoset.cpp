#include "infoset.h"
#include "Card.h"
#include "Enums/Moves.h"
#include <string>
#include <vector>

Infoset::Infoset() : currentCardBit(0), bettingHistoryBit(42), infoset(0) {}

void Infoset::addCard(const Card &card) {
    infoset |= (static_cast<__uint128_t>(card.encode()) << currentCardBit);
    currentCardBit += 6;
}

void Infoset::addMove(Moves move) {
    infoset |= (static_cast<__uint128_t>(move) << bettingHistoryBit);
    bettingHistoryBit += 3;
}

__uint128_t Infoset::getInfoset() const {
    return infoset;
}

std::string Infoset::toString() const {
    std::string result;
    result += "Cards: ";
    for (const Card &card : cards) {
        result += card.toString() + " ";
    }
    result += "| Moves: ";
    for (const Moves &move : moves) {
        result += moveString(move) + " ";
    }
    return result;
}