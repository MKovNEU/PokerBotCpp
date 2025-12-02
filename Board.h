#pragma once

#include <cstdint>
#include <vector>
#include "Card.h"
#include "infoset.h"
#include "Enums/Moves.h"
#include "Enums/Players.h"

using namespace std;

class Board {
private:
    //p1 is Button, p2 is BB
    vector<Card> p1Cards;
    vector<Card> p2Cards;
    vector<Card> p1Board;
    vector<Card> p2Board;
    Infoset p1Infoset; 
    Infoset p2Infoset;
    int potSize;
    int p1Stack;
    int p2Stack;
    int p1Committed;
    int p2Committed;
    int prevBet;
    int street; //0-preflop, 1-flop, 2-turn, 3-river
    Players currentPlayer; 
    Moves lastMove;
    bool terminal;

    void addFlopCardsToInfoset();
    void addTurnCardToInfoset();
    void addRiverCardToInfoset();

public:
    Board(const vector<Card> &player1Cards, const vector<Card> &player2Cards, 
            const vector<Card> &player1Board, const vector<Card> &player2Board) :
        p1Cards(player1Cards), p2Cards(player2Cards),
        p1Board(player1Board), p2Board(player2Board),
        p1Infoset(Infoset()), p2Infoset(Infoset()),
        potSize(1), p1Stack(100), p2Stack(99),
        p1Committed(0), p2Committed(1),
        prevBet(1), street(0),
        currentPlayer(Button),
        lastMove(RAISE1), terminal(false) {
            for (const Card &card : p1Cards) {
                p1Infoset.addCard(card);
            }
            for (const Card &card : p2Cards) {
                p2Infoset.addCard(card);
            }
        }

        
    void move(Moves move);

    Infoset getInfoset() const {
        if (currentPlayer == Button) {
            return p1Infoset;
        } else {
            return p2Infoset;
        }
    }

    bool isTerminal() const {
        return terminal;
    }

    int utilities() const;

    Players getCurrentPlayer() const {
        return currentPlayer;
    }

    int getNumActions() const;

    vector<Moves> getLegalActions() const;
};