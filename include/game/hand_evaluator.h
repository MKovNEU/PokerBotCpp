#pragma once 

#include "include/game/card.h"
#include <fstream>
#include <vector>
#include <stdexcept>


void loadHandRanks(const std::string &path);

int evaluateHand(const std::vector<Card> &hand, const std::vector<Card> &board);