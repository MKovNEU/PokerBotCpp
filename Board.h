#pragma once

#include <cstdint>
#include <vector>
#include "Card.h"
#include "infoset.h"
#include "Enums/Moves.h"
#include "Enums/Players.h"
#include "HandEvaluator.h"

using namespace std;

class Board {
private:
    vector<vector<Card>> cards;
    vector<Card> commCards;
    Players hero;
    vector<Infoset> infosets; //based on hero POV
    double potSize;
    vector<double> stacks;
    vector<bool> allIn;
    vector<double> handScores;
    Players currentPlayer; 
    bool terminal;
    double acesWin;

public:
    Board(const vector<vector<Card>> &playerCards, const vector<Card> &communityCards,
         const vector<double> &playerStacks, const Players traverser) :
        cards(playerCards), commCards(communityCards), hero(traverser), potSize(1.5), stacks(playerStacks), 
        handScores(4, 0), allIn(4, false), currentPlayer(UTG), terminal(false), acesWin(0)
        {
            for (int p = 0; p < 4; p++) {
                Infoset hInfoset;
                if (cards[p][0].suit == cards[p][1].suit) {
                    if (cards[p][0].rank > cards[p][1].rank) {
                        hInfoset.addCard(Card(cards[p][0].rank, CLUBS));
                        hInfoset.addCard(Card(cards[p][1].rank, CLUBS));
                    } else {
                        hInfoset.addCard(Card(cards[p][1].rank, CLUBS));
                        hInfoset.addCard(Card(cards[p][0].rank, CLUBS));
                    }
                } else {
                    if (cards[p][0].rank >= cards[p][1].rank) {
                        hInfoset.addCard(Card(cards[p][0].rank, CLUBS));
                        hInfoset.addCard(Card(cards[p][1].rank, DIAMONDS));
                    } else {
                        hInfoset.addCard(Card(cards[p][1].rank, CLUBS));
                        hInfoset.addCard(Card(cards[p][0].rank, DIAMONDS));
                    }
                }
                infosets.push_back(hInfoset);
            }
            handScores[0] = evaluateHand(cards[0], commCards);
            handScores[1] = evaluateHand(cards[1], commCards);
            handScores[2] = evaluateHand(cards[2], commCards);
            handScores[3] = evaluateHand(cards[3], commCards);
        }

        
    void move(Moves move);

    Infoset getInfoset(Players cp) const {
        return infosets[cp];
    }

    bool isTerminal() const {
        return terminal;
    }

    double utilities();

    Players getCurrentPlayer() const {
        return currentPlayer;
    }

    int getNumActions() const;

    vector<Moves> getLegalActions() const;
};