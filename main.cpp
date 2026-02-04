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

constexpr int MAX_ACTIONS = 5;
constexpr float REGRET_SCALE = 1.0f;
constexpr float STRATEGY_SCALE = 5e-2f;

const int ITERATIONS = 20000000;
//const float E = 0.05f;

vector<Card> deck = getDeck();

robin_hood::unordered_flat_map<__uint128_t, array<int16_t, MAX_ACTIONS>> regret;
robin_hood::unordered_flat_map<__uint128_t, array<int16_t, MAX_ACTIONS>> strategySum;

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
    //cout << "Original flop: ";
    //for (auto &c : board) cout << c.toString();
    //cout << endl;


    unordered_map<Suits, Suits> suitMap;
    unordered_map<Suits, int> count = {{CLUBS,0},{DIAMONDS,0},{HEARTS,0},{SPADES,0}};
    Suits nextSuit = CLUBS;

    if (hand1[0].rank == hand1[1].rank) {
        unordered_map<Suits, double> tempCount;
        for (int i = 0; i < board.size(); i++) {
            Suits suit = board[i].suit;
            if (tempCount.find(suit) == tempCount.end()) {
                tempCount[suit] = 1 + i / 10.0;
            } else tempCount[suit]++;
        }
        if (tempCount[hand1[0].suit] >= tempCount[hand1[1].suit]) {
            suitMap[hand1[0].suit] = CLUBS;
            suitMap[hand1[1].suit] = DIAMONDS;
        } else {
            suitMap[hand1[0].suit] = DIAMONDS;
            suitMap[hand1[1].suit] = CLUBS;
        }
        nextSuit = HEARTS;
    }

        
    for (const Card &card : hand1) {
        if (suitMap.find(card.suit) == suitMap.end()) {
            suitMap[card.suit] = nextSuit;
            nextSuit = Suits(nextSuit + 1);
        }
        newHand1.push_back(Card(card.rank, suitMap[card.suit]));
    }
    
    // Count flop suits
    vector<Card> flop = vector<Card>(board.begin(), board.begin() + 3);
    for (const Card &card : flop) {
        count[card.suit]++;
    }
    
    // Build suit counts for unmapped suits, in flop order
    vector<pair<Suits, int>> suitCounts;
    for (const Card &card : flop) {
        Suits s = card.suit;
        if (suitMap.find(s) == suitMap.end()) {
            // Check if already added
            bool found = false;
            for (auto &p : suitCounts) {
                if (p.first == s) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                suitCounts.push_back({s, count[s]});
            }
        }
    }
    
    stable_sort(suitCounts.begin(), suitCounts.end(), [](const pair<Suits, int> &a, const pair<Suits, int> &b) {
        return a.second > b.second;
    });
    /*
    cout << "suitCounts: ";
    for (auto &sc : suitCounts) {
        cout << "(" << sc.first << "," << sc.second << ") ";
    }
    cout << endl;
    */
    // Map remaining suits
    for (const auto &sc : suitCounts) {
        suitMap[sc.first] = nextSuit;
        nextSuit = Suits(nextSuit + 1);
    }
    
    // Map board
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
    
    return {newHand1, newFlop};
}

void testIso() {
   // Situation 1: A♥K♦ on 2♥5♥7♣
// Offsuit hand, two-tone flop where each hand card matches one flop suit
// Should all map to same thing - pocket pair with rainbow flop
vector<Card> h1 = {Card(ACE, HEARTS), Card(ACE, DIAMONDS)};
vector<Card> b1 = {Card(TWO, CLUBS), Card(FIVE, SPADES), Card(SEVEN, HEARTS)};

vector<Card> h2 = {Card(ACE, CLUBS), Card(ACE, SPADES)};
vector<Card> b2 = {Card(TWO, HEARTS), Card(FIVE, DIAMONDS), Card(SEVEN, CLUBS)};

// Should also be identical

auto res1 = isomorph(h1, b1);
auto res2 = isomorph(h2, b2);

cout << "Situation 1: ";
for (const Card &card : res1[0]) cout << card.toString();
cout << " | ";
for (const Card &card : res1[1]) cout << card.toString();
cout << "\n";

cout << "Situation 2: ";
for (const Card &card : res2[0]) cout << card.toString();
cout << " | ";
for (const Card &card : res2[1]) cout << card.toString();
cout << "\n";
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
    //boardCards.push_back(deck[7]); flop only version
    //boardCards.push_back(deck[8]); 
    vector<vector<Card>> handBoardPlayer1 = isomorph(player1Hand, boardCards);
    vector<vector<Card>> handBoardPlayer2 = isomorph(player2Hand, boardCards);
    return {handBoardPlayer1[0], handBoardPlayer1[1], handBoardPlayer2[0], handBoardPlayer2[1]};
}

vector<float> getProb(const Board &board, const int numActions) {
    __uint128_t infoset = board.getInfoset().getInfoset();
    int16_t vals[MAX_ACTIONS] = { 0 };
    vector<float> strategy(numActions); 
    if (regret.find(infoset) != regret.end()) {
        for (int i = 0; i < numActions; i++) {
            vals[i] = regret[infoset][i];
        }
    }
    int normalizingSum = 0;
    for (int16_t val : vals) {normalizingSum += ((val > 0) ? val : 0);}
    if (normalizingSum > 0) {
        for (int i = 0; i < numActions; i++)
            strategy[i] = ((vals[i] > 0) ? vals[i] : 0) / normalizingSum;
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

int play(Board &board, Players traverser, int depth) {
    //cout << depth << " ";
    if (board.isTerminal()) {
        return board.utilities();
    }

    Players currentPlayer = board.getCurrentPlayer(); 
    vector<Moves> actions = board.getLegalActions();

    //gets current strategy profile
    vector<float> strategy = getProb(board, actions.size());
    //vector<float> strategy;
    //for (float val : rawStrategy) { strategy.push_back((1-E) * val + E / rawStrategy.size()); }

    //opponent, sample one action
    if (currentPlayer != traverser) {
        int actionIndex  = weightedRandomChoice(strategy);
        board.move(actions[actionIndex]);
        return play(board, traverser, depth + 1);
    }

    //traverser, play all actions
    __uint128_t key = board.getInfoset().getInfoset();
    int numActions = actions.size();

    vector<int> actionUtils(numActions);
    int util = 0;

    for (int i = 0; i < numActions; i++) {
        Board nextBoard = board;
        nextBoard.move(actions[i]);

        actionUtils[i] = play(
            nextBoard,
            traverser,
            depth + 1
        );

        util += strategy[i] * actionUtils[i];
    }

    //regret and strategy sum update
    if (regret.find(key) == regret.end()) {
        auto &r = regret[key];
        auto &s = strategySum[key];

        memset(r.data(), 0, numActions * sizeof(int16_t));
        memset(s.data(), 0, numActions * sizeof(int16_t));
    }
    for (int i = 0; i < numActions; i++) {
    float delta = actionUtils[i] - util;
    int delta_q = (int)round(delta / REGRET_SCALE);

    int16_t newRegret = regret[key][i] + delta_q;
    regret[key][i] = max((int16_t)0, min(newRegret, (int16_t)32767));

    int sDelta = (int)round(strategy[i] / STRATEGY_SCALE);
    strategySum[key][i] = min(max(strategySum[key][i] + sDelta, 0), 32767);
}

    return util;
}

int main() {
    /*
    loadHandRanks("HandRanks.dat");
    random_device rd;
    mt19937 g(rd());
    for (int i = 0; i < ITERATIONS; i++) {
        if (i % 200000 == 0) {
            cout << "Size of dataset: " << regret.size() << endl;
        }
        shuffle(deck.begin(), deck.end(), g);
        vector<vector<Card>> preparedCards = prepareCards();
        Board board1(preparedCards[0], preparedCards[2], 
                    preparedCards[1], preparedCards[3], BUTTON);
        play(board1, BUTTON, 0);
        Board board2(preparedCards[2], preparedCards[0], 
                    preparedCards[3], preparedCards[1], BB);
        play(board2, BB, 0);
    }
    printStrategy();
    return 0;
    */
   testIso();
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
    ofstream file("output2.txt");
    for (const auto &entry : strategySum) {
        //cout << "\n";
        __uint128_t infoset = entry.first;
        string infosetS = decode(infoset);
        vector<int16_t> vals;
        for (const auto &val : entry.second) {
            vals.push_back(val);
        }
        int normalizingSum = 0.0f;
        for (int val : vals) {normalizingSum += val;}
        //cout << (normalizingSum) << " ";
        file.write(infosetS.c_str(), infosetS.size());
        for (int val : vals) {
            if (normalizingSum > 0) {
                file << (float)val / normalizingSum << " ";
                //cout << val << " ";
            } else {
                file << 1.0f / vals.size() << " ";
            }
        }
        file << endl;
    }
}