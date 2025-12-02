#include <iostream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include "Enums/Ranks.h"
#include "Enums/Suits.h"
#include "Enums/Moves.h"
#include "Board.h"
#include "Card.h"
#include "infoset.h"
#include "HandEvaluator.h"

using namespace std;

vector<Card> getDeck();
vector<vector<Card>> isomorph(const vector<Card> &hand1, const vector<Card> &board);
string decode(__uint128_t infoset);
void printStrategy();

const int ITERATIONS = 1000000;
const float E = 0.05f;

vector<Card> deck = getDeck();
unordered_map<__uint128_t, vector<float>> regret;
unordered_map<__uint128_t, vector<float>> strategySum;

vector<Card> getDeck() {
    vector<Card> deck;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck.push_back(Card(Ranks(r), Suits(s)));
        }
    }
    return deck;
}

vector<vector<Card>> isomorph(const vector<Card> &hand1, const vector<Card> &board){
    vector<Card> newHand1;
    vector<Card> newBoard;

    unordered_map<Suits, Suits> suitMap;
    unordered_map<Suits, int> count = {{CLUBS,0},{DIAMONDS,0},{HEARTS,0},{SPADES,0}};
    Suits nextSuit = CLUBS;
    for (const Card &card : hand1) {
        if (suitMap.find(card.suit) == suitMap.end()) {
            suitMap[card.suit] = nextSuit;
            nextSuit = Suits(nextSuit + 1);
        }
        newHand1.push_back(Card(card.rank, suitMap[card.suit]));
    }
    vector<Card> flop = vector<Card>(board.begin(), board.begin() + 3);
    for (const Card &card : flop) {
        count[card.suit]++;
    }
    vector<pair<Suits, int>> suitCounts;
    for (Suits s = CLUBS; s <= SPADES; s = Suits(s + 1)) {
        if (suitMap.find(s) == suitMap.end()) {suitCounts.push_back({s, count[s]});}
    }
    sort(suitCounts.begin(), suitCounts.end(), [](const pair<Suits, int> &a, const pair<Suits, int> &b) {
        return a.second > b.second;
    });
    if (suitCounts.size() == 2){
        suitMap[suitCounts[0].first] = HEARTS;
        suitMap[suitCounts[1].first] = SPADES;
    } else {
        suitMap[suitCounts[0].first] = DIAMONDS;
        suitMap[suitCounts[1].first] = HEARTS;
        suitMap[suitCounts[2].first] = SPADES;
    }
    for (const Card &card : board) {
        newBoard.push_back(Card(card.rank, suitMap[card.suit]));
    }
    vector<Card> newFlop = vector<Card>(newBoard.begin(), newBoard.begin() + 3);
    sort(newHand1.begin(), newHand1.end(), [](const Card &a, const Card &b) {
        return a.encode() < b.encode();
    });
    sort(newFlop.begin(), newFlop.end(), [](const Card &a, const Card &b) {
        return a.encode() < b.encode();
    });
    newFlop.push_back(newBoard[3]);
    newFlop.push_back(newBoard[4]);
    return {newHand1, newFlop};
}

vector<vector<Card>> prepareCards() {
    vector<Card> player1Hand(deck.begin(), deck.begin() + 2);
    sort(player1Hand.begin(), player1Hand.end(), [](const Card &a, const Card &b) {
        return a.encode() < b.encode();
    });
    vector<Card> player2Hand(deck.begin() + 2, deck.begin() + 4);
    sort(player2Hand.begin(), player2Hand.end(), [](const Card &a, const Card &b) {
        return a.encode() < b.encode();
    });
    vector<Card> boardCards(deck.begin() + 4, deck.begin() + 7);
    sort(boardCards.begin(), boardCards.end(), [](const Card &a, const Card &b) {
        return a.encode() < b.encode();
    });
    boardCards.push_back(deck[7]);
    boardCards.push_back(deck[8]);
    vector<vector<Card>> handBoardPlayer1 = isomorph(player1Hand, boardCards);
    vector<vector<Card>> handBoardPlayer2 = isomorph(player2Hand, boardCards);
    return {handBoardPlayer1[0], handBoardPlayer1[1], handBoardPlayer2[0], handBoardPlayer2[1]};
}

vector<float> getProb(Board board) {
    __uint128_t infoset = board.getInfoset().getInfoset();
    int numActions = board.getNumActions();
    if (regret.find(infoset) == regret.end()) {
        regret[infoset] = vector<float>(numActions, 0.0f);
    }
    vector<float> strategy = regret[infoset];
    float normalizingSum = 0.0f;
    for (float val : strategy) {normalizingSum += max(0.0f, val);}
    if (normalizingSum > 0) {
        for (int i = 0; i < strategy.size(); i++)
            strategy[i] = max(strategy[i], 0.0f) / normalizingSum;
        return strategy;
    } else {
        return vector<float>(numActions, 1.0f / numActions);
    }
}

int weightedRandomChoice(const vector<float> &strategy) {
    static thread_local mt19937 gen(random_device{}());  // seeded once
    uniform_real_distribution<float> dis(0.0f, 1.0f);
    float r = dis(gen);
    float cumulative = 0.0f;
    for (size_t i = 0; i < strategy.size(); i++) {
        cumulative += strategy[i];
        if (r <= cumulative) return i;
    }
    return strategy.size() - 1;
}

int play(Board &board, float p1Prob, float p2Prob) {
    if (board.isTerminal()) {
        return board.utilities();
    }
    // Get current strategy with exploration
    //cout << "Infoset: " << decode(board.getInfoset().getInfoset()) << endl;
    vector<float> rawStrategy = getProb(board);
    vector<float> strategy;
    for (float val : rawStrategy) {
        strategy.push_back((1-E) * val + E / rawStrategy.size());
    }
    // Select action based on strategy
    int actionIndex  = weightedRandomChoice(strategy);
    Moves action = board.getLegalActions()[actionIndex];
    //cout << "Num actions: " << board.getNumActions() <<  "Chosen action: " << moveString(action) << endl;
    float actionProb = strategy[actionIndex];
    // Adds to strategy sum
    __uint128_t key = board.getInfoset().getInfoset();
    float playerReach = (board.getCurrentPlayer() == Button) ? p1Prob : p2Prob;
    if (strategySum.find(key) == strategySum.end()) {
        strategySum[key] = vector<float>(strategy.size(), 0.0f);
    }
    for (int i = 0; i < strategy.size(); i++) {
        strategySum[key][i] += playerReach * strategy[i];
    }
    // Play the action
    board.move(action);

    float newP1Prob = p1Prob;
    float newP2Prob = p2Prob;
    if (board.getCurrentPlayer() == Button) {newP1Prob *= actionProb;}
    else {newP2Prob *= actionProb;}
    // Recursively play the next state
    int util = play(board, newP1Prob, newP2Prob);
    // Compute regrets
    int counterfactualValue = (board.getCurrentPlayer() == Button) ? util / newP1Prob : -util / newP2Prob;
    if (regret.find(key) == regret.end()) {
        regret[key] = vector<float>(strategy.size(), 0.0f);
    }
    for (int i = 0; i < strategy.size(); i++) {
        float regretValue = counterfactualValue * ( (i == actionIndex ? 1.0f : 0.0f) - strategy[i]);
        if (board.getCurrentPlayer() == Button) {
            regret[key][i] += regretValue * p2Prob;
        } else {
            regret[key][i] += regretValue * p1Prob;
        }
    }
    return util;
}

int main() {
    loadHandRanks("HandRanks.dat");
    random_device rd;
    mt19937 g(rd());
    for (int i = 0; i < ITERATIONS; i++) {
        if (i % 100000 == 0) {
            cout << "Size of dataset: " << regret.size() << endl;
        }
        shuffle(deck.begin(), deck.end(), g);
        vector<vector<Card>> preparedCards = prepareCards();
        Board board(preparedCards[0], preparedCards[2], 
                    preparedCards[1], preparedCards[3]);
        play(board, 1.0, 1.0);
    }
    printStrategy();
    return 0;
}

string decode(__uint128_t infoset) {
    Infoset temp;
    size_t cardBit = 0;
    size_t moveBit = 42;
    string result;
    result += "Cards: ";
    for (int i = 0; i < 7; i++) {
        uint8_t cardEncoded = (infoset >> cardBit) & 0x3F;
        if (cardEncoded == 0) {break;}
        Ranks rank = Ranks((cardEncoded - 1) / 4);
        Suits suit = Suits((cardEncoded - 1) % 4);
        Card card(rank, suit);
        result += card.toString() + " ";
        cardBit += 6; 
    }
    result += "| Moves: ";
    while (moveBit < 128) {
        uint8_t moveEncoded = (infoset >> moveBit) & 0x07;
        if (moveEncoded > ALL_IN || moveEncoded == 0) {break;}
        Moves move = Moves(moveEncoded);
        result += moveString(move) + " ";
        moveBit += 3;
    }
    return result;
}

void printStrategy() {
    ofstream file("output.txt");
    for (const auto &entry : strategySum) {
        __uint128_t infoset = entry.first;
        string infosetS = decode(infoset);
        const vector<float> &strategy = entry.second;
        float normalizingSum = 0.0f;
        for (float val : strategy) {normalizingSum += val;}
        file.write(infosetS.c_str(), infosetS.size());
        for (float val : strategy) {
            if (normalizingSum > 0) {
                file << val / normalizingSum << " ";
            } else {
                file << 1.0f / strategy.size() << " ";
            }
        }
        file << endl;
    }
}