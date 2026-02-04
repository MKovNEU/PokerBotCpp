#include "Board.h"
#include "HandEvaluator.h"

void Board::move(Moves move) {
    if (move == FOLD) {
        lastMove = FOLD;
        terminal = true;
        currentPlayer = (currentPlayer == BUTTON) ? BB : BUTTON;
    } else if (move == CALL) {
        if (lastMove != CALL && lastMove != NULL_MOVE) { //Calling a bet, moves money only
            int toCall = prevBet - (currentPlayer == hero ? heroCommittedStreet : villainCommittedStreet);
            if (currentPlayer == hero) {
                heroStack -= toCall;
                heroCommittedStreet += toCall;
                heroCommittedTotal += toCall;
            } else {
                villainStack -= toCall;
                villainCommittedStreet += toCall;
                villainCommittedTotal += toCall;
            }
            potSize += toCall;
        }
        if (lastMove == NULL_MOVE) { //first check
            lastMove = CALL;
            currentPlayer = (currentPlayer == BUTTON) ? BB : BUTTON;
        }
        else { //second check or calling a bet
            street++;
            prevBet = 0;
            numBets = 0;
            heroCommittedStreet = 0;
            villainCommittedStreet = 0;
            /*
            if (street > 3 || lastMove == ALL_IN) { //hand over
                terminal = true;
            }*/
            if (street > 1 || lastMove == ALL_IN) { //hand over, flop only version
                terminal = true;
            } else {
                lastMove = NULL_MOVE;
                //Add board cards to infosets
                if (street == 1) {
                    addFlopCardsToInfoset();
                } else if (street == 2) {
                    addTurnCardToInfoset();
                } else if (street == 3) {
                    addRiverCardToInfoset();
                }
            }
            currentPlayer = BB; //BB is first to act on all streets after preflop
        }
    } else { //raise 
        numBets++;
        int newBet = 0;
        int currCommited = (currentPlayer == hero ? heroCommittedStreet : villainCommittedStreet);
        int toCall = prevBet - currCommited;
        int effStack;
        effStack = min(heroStack + heroCommittedStreet, villainStack + villainCommittedStreet);
        if (move == RAISE1) {
            //2.5x, where 2x is the min allowed raise. 
            newBet = (prevBet == 0) ? int(0.4 * potSize) : (int(2.5 * toCall) + currCommited);
        } else if (move == RAISE2) {
            //3.5x
            newBet = (prevBet == 0) ? int(0.8 * potSize) : (int(3.5 * toCall) + currCommited);
        } else if (move == ALL_IN) {
            newBet = effStack; 
        }
        if (currentPlayer == hero) {
            heroStack -= (newBet - currCommited);
            heroCommittedStreet += (newBet - currCommited);
            heroCommittedTotal += (newBet - currCommited);
        } else {
            villainStack -= (newBet - currCommited);
            villainCommittedStreet += (newBet - currCommited);
            villainCommittedTotal += (newBet - currCommited);
        }
        potSize += (newBet - currCommited);
        prevBet = newBet;
        lastMove = move;
        currentPlayer = (currentPlayer == BUTTON) ? BB : BUTTON;
    }
    //Update infosets
    if (!terminal) hInfoset.addMove(move);
}

int Board::utilities() const {
    if (lastMove == FOLD) {
        return (currentPlayer == hero) ? villainCommittedTotal : -heroCommittedTotal;
    } else {
        //Showdown
        int heroBest = evaluateHand(heroCards, hCommCards);
        int villainBest = evaluateHand(villainCards, vCommCards);
        //lower score is better hand. 
        if (heroBest < villainBest) {
            return villainCommittedTotal;
        } else if (heroBest > villainBest) {
            return -heroCommittedTotal;
        } else {
            return 0; //split pot
        }
    }
}

vector<Moves> Board::getLegalActions() const {
    vector<Moves> actions;

    int currCommited = (currentPlayer == hero ? heroCommittedStreet: villainCommittedStreet);
    int effStack = min(heroStack + heroCommittedStreet, villainStack + villainCommittedStreet);
    int toCall = prevBet - currCommited;

    if (prevBet == 0) {
        actions.push_back(CALL); // check

        if (effStack * 5 > potSize * 2)
            actions.push_back(RAISE1);

        if (effStack * 5 > potSize * 4)
            actions.push_back(RAISE2);

        actions.push_back(ALL_IN);
    } else {
        actions.push_back(FOLD);
        actions.push_back(CALL);

        if (effStack > (2.5 * toCall + currCommited) && numBets < 3)
            actions.push_back(RAISE1);

        if (effStack > (3.5 * toCall + currCommited) && numBets < 3)
            actions.push_back(RAISE2);

        if (effStack > prevBet)
            actions.push_back(ALL_IN);
    }
    return actions;
}

int Board::getNumActions() const {
    return getLegalActions().size();
}

void Board::addFlopCardsToInfoset() {
    if (street == 1) {
        for (int i = 0; i < 3; i++) {
            hInfoset.addCard(hCommCards[i]);
        }
    }
}
void Board::addTurnCardToInfoset() {
    if (street == 2) {
        hInfoset.addCard(hCommCards[3]);
    }
}
void Board::addRiverCardToInfoset() {
    if (street == 3) {
        hInfoset.addCard(hCommCards[4]);
    }
}

