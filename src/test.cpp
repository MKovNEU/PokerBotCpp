#include <vector>
#include <iostream>
#include "include/solver/buckets.hpp"
#include "include/game/card.h"
#include "include/enums/ranks.h"
#include "include/enums/suits.h"

std::vector<Card> hand = {Card(ACE, CLUBS), Card(SIX, CLUBS)};

std::vector<Card> flop = {Card(TEN, CLUBS), Card(NINE, SPADES), Card(SEVEN, SPADES)};

int main() {
    //int temp = rawIndex(hand, flop); 
    int temp = isomorph(hand);
    std::cout << temp << std::endl;
    
    return 0;
}