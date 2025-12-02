#include "HandEvaluator.h"
#include <vector>

using namespace std;

static unsigned int HR[32487834];

void loadHandRanks(const std::string &path) {
    ifstream file(path, std::ios::binary);
    if (!file) throw runtime_error("Cannot open handRanks.dat");

    file.read(reinterpret_cast<char*>(HR), sizeof(HR));

    if (!file) throw runtime_error("Failed reading handRanks.dat");
}

int evaluateHand(const std::vector<Card> &hand, const std::vector<Card> &board) {
    vector<Card> allCards;
    for (const Card &card : hand) {
        allCards.push_back(card);
    }
    for (const Card &card : board) {
        allCards.push_back(card);
    }
    int cards[7];
    for (size_t i = 0; i < allCards.size(); i++) {
        cards[i] = allCards[i].encode();
    }
    int p = 53;
    for (size_t i = 0; i < allCards.size(); i++) {
        p = HR[p + cards[i]];
    }
    return p;
}
