#pragma once

#include <cstdint>
#include <vector>
#include "include/game/card.h"
#include "include/solver/infoset.h"
#include "include/enums/moves.h"
#include "include/game/undo_info.h"
#include "include/enums/players.h"
#include "include/solver/buckets.hpp"

using namespace std;

constexpr int MAX_ACTIONS = 5;
class Board {
private:
    vector<Card> heroCards;
    vector<Card> villainCards;
    vector<Card> commCards;
    vector<int> heroIndices;
    vector<int> villainIndices;
    vector<int> boardIndices;
    int privateKeyCount; //number of private keys for this street
    Players hero;
    int potSize;
    int heroStack;
    int villainStack;
    int heroCommittedTotal;
    int villainCommittedTotal;
    int heroCommittedStreet;
    int villainCommittedStreet;
    int prevBet;
    int street; //0-preflop, 1-flop, 2-turn, 3-river
    Players currentPlayer; 
    Moves lastMove;
    bool terminal;
    int numBets;
    bool firstMove;
    bool streetChange;

    //void addFlopCardsToInfoset();

    //void removeFlopCardsFromInfoset();

public:
    Board(const vector<Card> &player1Cards, const vector<Card> &player2Cards, 
            const vector<Card> &board, const Players traverser) :
        heroCards(player1Cards), villainCards(player2Cards), commCards(board), hero(traverser), potSize(1), 
        heroStack((traverser == BUTTON) ? 100 : 99), villainStack((traverser == BUTTON) ? 99 : 100),
        heroCommittedTotal((traverser == BUTTON) ? 0 : 1), villainCommittedTotal((traverser == BUTTON) ? 1 : 0),
        heroCommittedStreet((traverser == BUTTON) ? 0 : 1), villainCommittedStreet((traverser == BUTTON) ? 1 : 0),
        prevBet(1), street(0), firstMove(true),
        currentPlayer(BUTTON), numBets(1), streetChange(false),
        lastMove(RAISE1), terminal(false) {
            heroIndices.push_back(isomorph(player1Cards));
            villainIndices.push_back(isomorph(player2Cards));
            for (int i = 3; i <= 5; i++) {
                heroIndices.push_back(rawIndex(player1Cards, std::vector<Card>(board.begin(), board.begin() + i)));
                villainIndices.push_back(rawIndex(player1Cards, std::vector<Card>(board.begin(), board.begin() + i)));          
            }
            boardIndices.push_back(getBucket(isomorph(player1Cards, std::vector<Card>(board.begin(), board.begin() + 3))[1])); //flop bucket
            boardIndices.push_back(rawIndex(board[4], std::vector<Card>(board.begin(), board.begin() + 3))); //turn bucket
            boardIndices.push_back(rawIndex(board[5], std::vector<Card>(board.begin(), board.begin() + 4))); //river bucket
        }

        
    UndoInfo move(Moves move);

    void unmakeMove(const UndoInfo &undo);

    bool isTerminal() const {
        return terminal;
    }

    double utilities() const;

    Players getCurrentPlayer() const {
        return currentPlayer;
    }

    int getLegalActions(std::array<Moves, MAX_ACTIONS>& outActions) const;

    int getNumActions() const;

    int getStreetIndex() const; //index with one-to-one mapping to a street, ie for flop, index is from 0 - 1154

    int getPrivateHandIndex() const; //index of currentPlayers hand, one-to-on map, ie 0 -> 2s 2d, 168 -> As Ad;

    int getPrivateKeyCount() const; //number of private keys that can be matchted to the public key, ie no cards in common

    bool switchedStreets() const; //did the street just change and cards not dealt.

    int getStreetOutcomes() const; //How many possible outcomes for chance node, ie 50 for flop, etc
};