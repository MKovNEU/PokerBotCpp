#include "Board.h"
#include "HandEvaluator.h"

void Board::move(Moves move) {
    if (move == FOLD) {
        lastMove = FOLD;
        terminal = true;
        currentPlayer = (currentPlayer == Button) ? BB : Button;
    } else if (move == CALL) {
        if (lastMove != CALL && lastMove != NULL_MOVE) { //Calling a bet. 
            int toCall = prevBet;
            if (currentPlayer == Button) {
                p1Stack -= toCall;
                p1Committed += toCall;
            } else {
                p2Stack -= toCall;
                p2Committed += toCall;
            }
            potSize += toCall;
        }
        if (lastMove == NULL_MOVE) {
            lastMove = CALL; //first check
            currentPlayer = (currentPlayer == Button) ? BB : Button;
        }
        else { //second check or calling a bet. 
            //Move to next street
            street++;
            prevBet = 0;
            if (street > 3 || lastMove == ALL_IN) { //hand over
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
        int raiseAmount = 0;
        int toCall = prevBet;
        int effStack = min(p1Stack, p2Stack);
        if (move == RAISE1) {
            raiseAmount = (prevBet = 0) ? int(0.4 * potSize) : int(2.5 * prevBet) - toCall;
        } else if (move == RAISE2) {
            raiseAmount = (prevBet = 0) ? int(0.8 * potSize) : int(3.5 * prevBet) - toCall;
        } else if (move == ALL_IN) {
            raiseAmount = effStack;
        }
        if (currentPlayer == Button) {
            p1Stack -= (toCall + raiseAmount);
            p1Committed += (toCall + raiseAmount);
        } else {
            p2Stack -= (toCall + raiseAmount);
            p2Committed += (toCall + raiseAmount);
        }
        potSize += (toCall + raiseAmount);
        prevBet = raiseAmount;
        lastMove = move;
        currentPlayer = (currentPlayer == Button) ? BB : Button;
    }
    //Update infosets
    p1Infoset.addMove(move);
    p2Infoset.addMove(move);
}

int Board::utilities() const {
    if (lastMove == FOLD) {
        return (currentPlayer == Button) ? p2Committed : -p1Committed;
    } else {
        //Showdown
        int p1Best = evaluateHand(p1Cards, p1Board);
        int p2Best = evaluateHand(p2Cards, p2Board);
        //lower score is better hand. 
        if (p1Best < p2Best) {
            return p2Committed;
        } else if (p2Best < p1Best) {
            return -p1Committed;
        } else {
            return 0; //split pot
        }
    }
}

vector<Moves> Board::getLegalActions() const {
    vector<Moves> actions;
    int effStack = min(p1Stack, p2Stack);
    //First to bet
    if (prevBet == 0) {
        double ratio = double(effStack) / potSize;
        actions.push_back(CALL); //check
        if (ratio > 0.4) { //if less than this, all in is only option
            actions.push_back(RAISE1);
        }
        if (ratio > 0.8) { // if less than this, all in or smaller raise is only option
            actions.push_back(RAISE2);
        }
        actions.push_back(ALL_IN);
    } else { //facing a bet
        actions.push_back(FOLD);
        actions.push_back(CALL);
        // effStack changed to reflect prev bet. 
        if (effStack > prevBet * 2.5) { //if less than this, all in is only option
            actions.push_back(RAISE1);
        }
        if (effStack > prevBet * 3.5) { //if less than this, all in or smaller raise is only option
            actions.push_back(RAISE2);
        }
        //cannot jam if prev bet is already all in, considered call
        if (effStack > 0) {
            actions.push_back(ALL_IN);
        }
    }
    return actions;
}

int Board::getNumActions() const {
    return getLegalActions().size();
}

void Board::addFlopCardsToInfoset() {
    if (street == 1) {
        for (int i = 0; i < 3; i++) {
            p1Infoset.addCard(p1Board[i]);
            p2Infoset.addCard(p2Board[i]);
        }
    }
}
void Board::addTurnCardToInfoset() {
    if (street == 2) {
        p1Infoset.addCard(p1Board[3]);
        p2Infoset.addCard(p2Board[3]);
    }
}
void Board::addRiverCardToInfoset() {
    if (street == 3) {
        p1Infoset.addCard(p1Board[4]);
        p2Infoset.addCard(p2Board[4]);
    }
}

