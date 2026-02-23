#include <iostream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include "include/enums/ranks.h"
#include "include/enums/suits.h"
#include "include/enums/moves.h"
#include "include/game/board.h"
#include "include/game/card.h"
#include "include/solver/infoset.h"
#include "include/game/hand_evaluator.h"
#include "lib/robin_hood/robin_hood.h"
#include "include/solver/buckets.hpp"
#include "include/game/undo_info.h"
#include "include/game/node.h"


using namespace std;

vector<Card> getDeck();
vector<vector<Card>> isomorph(const vector<Card> &hand1, const vector<Card> &board);
string decode(ll infoset);
void printPreflopStrategy(const PublicNode& root, int numActions);

const int ITERATIONS = 300000000;

int treeSize = 0;

//const float E = 0.05f;

vector<Card> deck = getDeck();

vector<Card> getDeck() {
    vector<Card> deck;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck.push_back(Card(Ranks(r), Suits(s)));
        }
    }
    return deck;
}

vector<vector<Card>> prepareCards() {
    vector<Card> player1Hand(deck.begin(), deck.begin() + 2);
    vector<Card> player2Hand(deck.begin() + 2, deck.begin() + 4);
    vector<Card> boardCards(deck.begin() + 4, deck.begin() + 9);
    return {player1Hand, player2Hand, boardCards};
}

std::array<float, MAX_ACTIONS> getProb(const PublicNode &node, int privateHandIndex, int numActions) {
    std::array<float, MAX_ACTIONS> strategy = {0.0f};
    float normalizingSum = 0.0f;

    for (int i = 0; i < numActions; i++) {
        float positiveRegret = std::max(0.0f, node.regrets[numActions * privateHandIndex + i]);
        strategy[i] = positiveRegret;
        normalizingSum += positiveRegret;
    }

    if (normalizingSum > 0) {
        for (int i = 0; i < numActions; i++) {
            strategy[i] /= normalizingSum;
        }
    } else {
        // Fallback to uniform random
        float uniformProb = 1.0f / numActions;
        for (int i = 0; i < numActions; i++) {
            strategy[i] = uniformProb;
        }
    }
    return strategy;
}

int weightedRandomChoice(const std::array<float, MAX_ACTIONS>& strategy, int numActions) {
    static thread_local mt19937 gen(random_device{}());
    uniform_real_distribution<float> dis(0.0, 1.0);
    float r = dis(gen);
    float cumulative = 0.0f;
    for (int i = 0; i < numActions; i++) {
        cumulative += strategy[i];
        if (r <= cumulative) return i;
    }
    return numActions - 1;
}

double play(PublicNode &node, Board &board, Players traverser, int depth, int t) {

    if (node.type == CHANCE_NODE) {
        int index = board.getStreetIndex();
        if (node.children[index] == nullptr){
            treeSize++;
            node.children[index] = new PublicNode(ACTION_NODE, board.getPrivateKeyCount(), board.getNumActions());
        }
        return play(*node.children[index], board, traverser, depth+1, t);
    }

    Players currentPlayer = board.getCurrentPlayer(); 
    array<Moves, MAX_ACTIONS> actions;
    int numActions = board.getLegalActions(actions);
    int privateHandIndex = board.getPrivateHandIndex();
    //cout << privateHandIndex << "\n";

    std::array<float, MAX_ACTIONS> strategy = getProb(node, privateHandIndex, numActions);

    //opponent, sample one action
    if (currentPlayer != traverser) {
        for (int i = 0; i < numActions; i++) {
            node.strategySum[privateHandIndex * numActions + i] += float(t) * strategy[i];
        }

        int actionIndex  = weightedRandomChoice(strategy, numActions);
        UndoInfo undo = board.move(actions[actionIndex]);

        double util = 0.0;
        if (board.isTerminal()) {
            util = board.utilities();
        } else {
            if (node.children[actionIndex] == nullptr) {
                treeSize++;
                if (board.switchedStreets()) {
                    node.children[actionIndex] = new PublicNode(CHANCE_NODE, board.getStreetOutcomes());
                } else {
                    node.children[actionIndex] = new PublicNode(ACTION_NODE, board.getPrivateKeyCount(), board.getNumActions());
                }
            }
            util = play(*node.children[actionIndex], board, traverser, depth + 1, t);
        }

        board.unmakeMove(undo);
        return util;
    }

    //traverser, play all actions
    vector<double> actionUtils(numActions);
    double util = 0;

    for (int i = 0; i < numActions; i++) {
        UndoInfo undo = board.move(actions[i]);
        //Board newBoard = board;
        //newBoard.move2(actions[i]);

        if (board.isTerminal()) {
            actionUtils[i] = board.utilities();
        } else {
            if (node.children[i] == nullptr) {
                treeSize++;
                if (board.switchedStreets()) {
                    node.children[i] = new PublicNode(CHANCE_NODE, board.getStreetOutcomes());
                } else {
                    node.children[i] = new PublicNode(ACTION_NODE, board.getPrivateKeyCount(), board.getNumActions());
                }
            }
            actionUtils[i] = play(*node.children[i], board, traverser, depth + 1, t);
        }

        util += strategy[i] * actionUtils[i];
        board.unmakeMove(undo);
    }

    for (int i = 0; i < numActions; i++) {
        node.regrets[privateHandIndex * numActions + i] += float(t) * float(actionUtils[i] - util);
    }

    return util;
}

int main() {
    loadHandRanks("data/hand_ranks.dat");
    random_device rd;
    mt19937 g(rd());
    generateBuckets();
    PublicNode root(ACTION_NODE, 169, MAX_ACTIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        if (i % 300 == 0) {
            cout << "Percent Done : " << (i * 100.0 / ITERATIONS) << " | Size of dataset: " << treeSize << endl;
        }
        shuffle(deck.begin(), deck.end(), g);
        vector<vector<Card>> preparedCards = prepareCards();
        Board board1(preparedCards[0], preparedCards[1], 
                    preparedCards[2], BUTTON);
        play(root, board1, BUTTON, 0, i);
        Board board2(preparedCards[1], preparedCards[0], 
                    preparedCards[2],  BB);
        play(root,board2, BB, 0, i);
    }
    printPreflopStrategy(root, MAX_ACTIONS);
    return 0;
}


#include <iomanip> // For formatting the output nicely

void printPreflopStrategy(const PublicNode& root, int numActions) {
    ofstream file("data/preflop_strategy.txt");
    if (!file.is_open()) {
        cerr << "Error opening preflop strategy file!" << endl;
        return;
    }

    int numBuckets = 169; // Standard strategically distinct preflop hands

    file << "Preflop Button Strategy (Root Node)\n";
    file << "--------------------------------------------------\n";
    
    // Optional: Print header for actions if you know what they are
    // file << "Bucket\tFold\tCall\tRaise1\tRaise2\tAll-In\n";

    for (int b = 0; b < numBuckets; b++) {
        std::vector<float> strategy(numActions, 0.0f);
        float normalizingSum = 0.0f;

        // 1. Extract the strategy sum for this specific preflop hand (bucket 'b')
        for (int a = 0; a < numActions; a++) {
            // Use the exact same 1D flat array math from your traverser
            float sum = std::max(0.0f, root.strategySum[b * numActions + a]);
            strategy[a] = sum;
            normalizingSum += sum;
        }

        file << "Hand: " << indexToHandString(b) << " | ";

        // 2. Normalize and print the probabilities
        if (normalizingSum > 0) {
            for (int a = 0; a < numActions; a++) {
                file << std::fixed << std::setprecision(4) << (strategy[a] / normalizingSum) << "  ";
            }
        } else {
            // Fallback to uniform if this hand somehow never saw action (unlikely at root)
            for (int a = 0; a < numActions; a++) {
                file << std::fixed << std::setprecision(4) << (1.0f / numActions) << "  ";
            }
        }
        file << "\n";
    }

    file.close();
    cout << "Preflop strategy successfully written to data/preflop_strategy.txt" << endl;
}

/*
string decode(ll infoset) {
    Infoset temp;
    size_t cardBit = 0;
    size_t moveBit = 18; //2 cards + bucket
    string result;
    result += "Cards: ";
    for (int i = 0; i < 2; i++) {
        uint8_t cardEncoded = (infoset >> cardBit) & 0x3F;
        if (cardEncoded == 0) {break;}
        Ranks rank = Ranks((cardEncoded - 1) / 4);
        Suits suit = Suits((cardEncoded - 1) % 4);
        Card card(rank, suit);
        result += card.toString() + " ";
        cardBit += 6; 
    }
    result += "| Flop Bucket: " + to_string((infoset >> cardBit) & 0x3F) + " ";
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
    ofstream file("data/output.txt");
    for (const auto &entry : tree) {
        //cout << "\n";
        ll infoset = entry.first;
        string infosetS = decode(infoset);
        vector<float> vals;
        for (const auto &val : entry.second.strategySum) {
            vals.push_back(val);
        }
        float normalizingSum = 0.0f;
        for (float val : vals) {normalizingSum += val;}
        //cout << (normalizingSum) << " ";
        file.write(infosetS.c_str(), infosetS.size());
        for (float val : vals) {
            if (normalizingSum > 0) {
                file << val / normalizingSum << " ";
                //cout << val << " ";
            } else {
                file << 1.0f / vals.size() << " ";
            }
        }
        file << endl;
    }
}
*/