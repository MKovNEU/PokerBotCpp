#include <string>
#include <vector>
#include <iostream>
#include "include/solver/infoset.h"
#include "include/game/card.h"
#include "include/enums/moves.h"

Infoset::Infoset() : currentCardBit(0), bettingHistoryBit(18), infoset(0) {}

void Infoset::addCard(const Card &card) {
    if (card.encode() > 52 || card.encode() < 1) {
        std::cout << "bug here";
    }
    infoset |= (static_cast<u_int64_t>(card.encode()) << currentCardBit);
    currentCardBit += 6;
}

void Infoset::removeCard() {
    if (currentCardBit >= 6) {
        currentCardBit -= 6;
        infoset &= ~((static_cast<u_int64_t>(0x3F)) << currentCardBit);
    }
}

void Infoset::addBucket(int bucket) {
    infoset |= (static_cast<u_int64_t>(bucket) << currentCardBit);
    currentCardBit += 6;

}

void Infoset::addMove(Moves move) {
    infoset |= (static_cast<u_int64_t>(move) << bettingHistoryBit);
    bettingHistoryBit += 3;
}

void Infoset::popMove() {
    if (bettingHistoryBit >= 21) {
        bettingHistoryBit -= 3;
        u_int64_t mask = 0x7ULL;
        infoset &= ~(mask << bettingHistoryBit);
    }
}

ll Infoset::getInfoset() const {
    return infoset;
}
