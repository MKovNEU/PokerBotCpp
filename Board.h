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
    vector<Card> heroCards;
    vector<Card> villainCards;
    vector<Card> hCommCards; // comm cards based on hero abstraction
    vector<Card> vCommCards; // comm cards based on villian abstraction
    Players hero;
    Infoset hInfoset; //based on hero POV
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

    void addFlopCardsToInfoset();
    void addTurnCardToInfoset();
    void addRiverCardToInfoset();

public:
    Board(const vector<Card> &player1Cards, const vector<Card> &player2Cards, 
            const vector<Card> &player1Board, const vector<Card> &player2Board, const Players traverser) :
        heroCards(player1Cards), villainCards(player2Cards),
        hCommCards(player1Board), vCommCards(player2Board), hero(traverser),
        hInfoset(Infoset()), potSize(1), 
        heroStack((traverser == BUTTON) ? 100 : 99), villainStack((traverser == BUTTON) ? 99 : 100),
        heroCommittedTotal((traverser == BUTTON) ? 0 : 1), villainCommittedTotal((traverser == BUTTON) ? 1 : 0),
        heroCommittedStreet((traverser == BUTTON) ? 0 : 1), villainCommittedStreet((traverser == BUTTON) ? 1 : 0),
        prevBet(1), street(0),
        currentPlayer(BUTTON), numBets(1),
        lastMove(RAISE1), terminal(false) {
            for (const Card &card : heroCards) {
                hInfoset.addCard(card);
            }
        }

        
    void move(Moves move);

    Infoset getInfoset() const {
        return hInfoset;
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