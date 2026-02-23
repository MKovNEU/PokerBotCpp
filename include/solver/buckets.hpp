#pragma once

#include <unordered_map>
#include <array>
#include <vector>
#include "include/game/card.h"

struct ArrayCardHash; 

void generateBuckets();

int isomorph(const std::vector<Card> &hand);

std::vector<std::vector<Card>> isomorph(const std::vector<Card> &hand, const std::vector<Card> &flop);

int rawIndex(const std::vector<Card> &hand, const std::vector<Card> &board);

int rawIndex(const Card &card, const std::vector<Card> &board);

int getBucket(const std::vector<Card> &flop);

std::string indexToHandString(int index);
