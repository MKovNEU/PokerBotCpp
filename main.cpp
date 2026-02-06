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
#include "robin_hood/robin_hood.h"

using namespace std;

vector<Card> getDeck();
vector<vector<Card>> isomorph(const vector<Card> &hand1, const vector<Card> &board);
string decode(__uint128_t infoset);
void printStrategy();

constexpr int MAX_ACTIONS = 2;
constexpr float REGRET_SCALE = 1.0f;
constexpr float STRATEGY_SCALE = 5e-2f;

const int ITERATIONS = 50000000;
//const float E = 0.05f;

vector<Card> deck = getDeck();

robin_hood::unordered_flat_map<__uint128_t, array<double, MAX_ACTIONS>> regret;
robin_hood::unordered_flat_map<__uint128_t, array<double, MAX_ACTIONS>> strategySum;

vector<Card> getDeck() {
    vector<Card> deck;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck.push_back(Card(Ranks(r), Suits(s)));
        }
    }
    return deck;
}

vector<double> getProb(const Board &board, int numActions) {
    __uint128_t infoset = board.getInfoset(board.getCurrentPlayer()).getInfoset();
    vector<double> strategy(numActions); 
    
    auto it = regret.find(infoset);
    if (it == regret.end()) {
        return vector<double>(numActions, 1.0 / numActions);
    }

    double normalizingSum = 0.0;
    for (int i = 0; i < numActions; i++) {
        normalizingSum += (it->second[i] > 0) ? it->second[i] : 0;
    }

    if (normalizingSum > 0) {
        for (int i = 0; i < numActions; i++)
            strategy[i] = ((it->second[i] > 0) ? it->second[i] : 0) / normalizingSum;
    } else {
        fill(strategy.begin(), strategy.end(), 1.0 / numActions);
    }
    return strategy;
}

int weightedRandomChoice(const vector<double> &strategy) {
    static thread_local mt19937 gen(random_device{}());  // seeded once
    uniform_real_distribution<double> dis(0.0, 1.0);
    double r = dis(gen);
    double cumulative = 0.0;
    for (size_t i = 0; i < strategy.size(); i++) {
        cumulative += strategy[i];
        if (r <= cumulative) return (int)i;
    }
    return (int)(strategy.size() - 1);
}

double play(Board &board, Players traverser, int depth, int t) {
    //cout << depth << " ";
    if (board.isTerminal()) {
        return board.utilities();
    }

    Players currentPlayer = board.getCurrentPlayer(); 
    vector<Moves> actions = board.getLegalActions();
    __uint128_t key = board.getInfoset(currentPlayer).getInfoset();
    int numActions = actions.size();

    //gets current strategy profile
    vector<double> strategy = getProb(board, actions.size());
    //vector<float> strategy;
    //for (float val : rawStrategy) { strategy.push_back((1-E) * val + E / rawStrategy.size()); }

    //opponent, sample one action
    if (currentPlayer != traverser) {
        auto& s = strategySum[key];
        for (int i = 0; i < numActions; i++) {
            if (t > 1000000) { // Only start averaging after the bot stops being "random"
                s[i] += double(t) * strategy[i];
            }
        }
        int actionIndex  = weightedRandomChoice(strategy);
        board.move(actions[actionIndex]);
        return play(board, traverser, depth + 1, t);
    }

    //traverser, play all actions

    vector<double> actionUtils(numActions);
    double util = 0.0;

    for (int i = 0; i < numActions; i++) {
        Board nextBoard = board;
        nextBoard.move(actions[i]);

        actionUtils[i] = play(
            nextBoard,
            traverser,
            depth + 1,
            t
        );

        util += strategy[i] * actionUtils[i];
    }

    //regret update
    auto& r = regret[key];
    for (int i = 0; i < numActions; i++) {
        r[i] = max(0.0, r[i] + double(t) * (actionUtils[i] - util));
    }

    return util;
}

int main() {
    loadHandRanks("HandRanks.dat");
    random_device rd;
    mt19937 g(rd());
    int percent = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        if (i % 500000 == 0) {
            cout << "Percent Done: " << percent << " | " << "Size of dataset: " << regret.size() << endl;
            percent++;
        }
        shuffle(deck.begin(), deck.end(), g);
        vector<vector<Card>> hands;
        for (int i = 0; i < 4; i++) {
           hands.push_back(vector<Card>(deck.begin() + 2*i, deck.begin() + 2 * (i+1)));
        }
        vector<Card> commCards = vector<Card>(deck.begin() + 8, deck.begin() + 13);
        vector<double> stacks{30, 30, 29.5, 29};
        for (int p = 0; p < 4; p++) {
            Board board(hands, commCards, stacks, (Players)p);
            play(board, (Players)p, 0, i);
        }
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
        //cout << "\n";
        __uint128_t infoset = entry.first;
        string infosetS = decode(infoset);
        vector<double> vals;
        for (const auto &val : entry.second) {
            vals.push_back(val);
        }
        double normalizingSum = 0.0;
        for (double val : vals) {normalizingSum += val;}
        //cout << (normalizingSum) << " ";
        file.write(infosetS.c_str(), infosetS.size());
        for (double val : vals) {
            if (normalizingSum > 0) {
                file << val / normalizingSum << " | ";
                //cout << val << " ";
            } else {
                file << 1.0 / vals.size() << " | ";
            }
        }
        file << endl;
    }
}