#include <iostream>
#include <cmath>
#include "include/game/board.h"
#include "include/game/hand_evaluator.h"
#include "include/solver/buckets.hpp"
#include "include/solver/infoset.h"
#include "include/game/undo_info.h"

UndoInfo Board::move(Moves move) {
    UndoInfo undo;
    undo.firstMove = firstMove;
    undo.streetChange = streetChange;
    undo.lastMove = lastMove;
    undo.terminal = terminal;
    undo.currentPlayer = currentPlayer;
    undo.potSize = potSize;
    undo.prevBet = prevBet;
    undo.numBets = numBets;
    undo.street = street;
    undo.heroStack = heroStack;
    undo.villainStack = villainStack;
    undo.heroCommittedStreet = heroCommittedStreet;
    undo.villainCommittedStreet = villainCommittedStreet;
    undo.heroCommittedTotal = heroCommittedTotal;
    undo.villainCommittedTotal = villainCommittedTotal;
    
    // Default these to false, we set them true if specific branches hit
    //undo.infosetUpdated = false; 
    //undo.streetChange = false;

    if (streetChange) streetChange = false;
    if (move == FOLD) {
        lastMove = FOLD;
        terminal = true;
        currentPlayer = (currentPlayer == BUTTON) ? BB : BUTTON; //to ensure we know who folded
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
        if (lastMove == NULL_MOVE || firstMove) { //first check
            lastMove = CALL;
            currentPlayer = (currentPlayer == BUTTON) ? BB : BUTTON;
        }
        else { //second check or calling a bet
            streetChange = true;
            street++;
            if (numBets > 4) cout << "here";
            prevBet = 0;
            numBets = 0;
            heroCommittedStreet = 0;
            villainCommittedStreet = 0;
            if (street > 3 || lastMove == ALL_IN) { //hand over, flop only version
                lastMove = CALL;
                terminal = true;
            } else {
                lastMove = NULL_MOVE;
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
            newBet = max(1, (prevBet == 0) ? int(round(0.4 * potSize)) : (int(round(2.5 * toCall)) + currCommited));
        } else if (move == RAISE2) {
            //3.5x
            newBet = max(1, (prevBet == 0) ? int(round(0.8 * potSize)) : (int(round(3.5 * toCall)) + currCommited));
        } else if (move == ALL_IN) {
            newBet = max(1, effStack); 
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
    if (firstMove) firstMove = false;
    return undo;
}


double Board::utilities() const {
    if (lastMove == FOLD) {
        return (currentPlayer == hero) ? villainCommittedTotal : -heroCommittedTotal;
    } else {
        int heroBest = evaluateHand(heroCards, commCards);
        int villainBest = evaluateHand(villainCards, commCards);
        //higher score is better hand. 
        if (heroBest > villainBest) {
            return villainCommittedTotal;
        } else if (heroBest < villainBest) {
            return -heroCommittedTotal;
        } else {
            return 0; //split pot
        }
    }
}


int Board::getLegalActions(std::array<Moves, MAX_ACTIONS>& outActions) const{
    int currCommited = (currentPlayer == hero ? heroCommittedStreet: villainCommittedStreet);
    int effStack = min(heroStack + heroCommittedStreet, villainStack + villainCommittedStreet);
    int toCall = prevBet - currCommited;
    
    int count = 0;

    if (prevBet == 0) {
        outActions[count++] = CALL;

        if (effStack * 5 > potSize * 2)
            outActions[count++] = RAISE1;

        if (effStack * 5 > potSize * 4)
            outActions[count++] = RAISE2;

        outActions[count++] = ALL_IN;
    } else {
        outActions[count++] = FOLD;
        outActions[count++] = CALL;

        if (effStack * 2 - (5 * toCall) > 2 * currCommited && numBets < 3)
            outActions[count++] = RAISE1;

        if (effStack * 2 - (7 * toCall) > 2 * currCommited && numBets < 3)
            outActions[count++] = RAISE2;

        if (effStack > prevBet)
            outActions[count++] = ALL_IN;
    }
    return count;
}


void Board::unmakeMove(const UndoInfo& undo) {
    /*
    if (undo.infosetUpdated) {
        hInfoset.popMove(); 
        vInfoset.popMove();
    }

    if (undo.streetChange) {
        removeFlopCardsFromInfoset(); 
    }
    */

    firstMove = undo.firstMove;
    streetChange = undo.streetChange;
    lastMove = undo.lastMove;
    terminal = undo.terminal;
    currentPlayer = undo.currentPlayer;
    potSize = undo.potSize;
    prevBet = undo.prevBet;
    numBets = undo.numBets;
    street = undo.street;

    heroStack = undo.heroStack;
    villainStack = undo.villainStack;
    heroCommittedStreet = undo.heroCommittedStreet;
    villainCommittedStreet = undo.villainCommittedStreet;
    heroCommittedTotal = undo.heroCommittedTotal;
    villainCommittedTotal = undo.villainCommittedTotal;
}

/*
void Board::addFlopCardsToInfoset() {
    if (street == 1) {
        for (int i = 0; i < 2; i++) {
            hInfoset.removeCard();
            vInfoset.removeCard();
        }
        for (int i = 0; i < 2; i++) {
            hInfoset.addCard(isoHeroCards[i]);
            vInfoset.addCard(isoVillainCards[i]);
        }
        hInfoset.addBucket(getBucket(isoFlopCards));
        vInfoset.addBucket(getBucket(isoFlopCards));
    } else {
        std::cout << "SHUOLD NEVER GET HERE" << endl;
    }
}
*/

int Board::getNumActions() const {
    int currCommited = (currentPlayer == hero ? heroCommittedStreet: villainCommittedStreet);
    int effStack = min(heroStack + heroCommittedStreet, villainStack + villainCommittedStreet);
    int toCall = prevBet - currCommited;
    
    int count = 2;

    if (prevBet == 0) {
        if (effStack * 5 > potSize * 2)
            count++;

        if (effStack * 5 > potSize * 4)
            count++;

    } else {
        if (effStack * 2 - (5 * toCall) > 2 * currCommited && numBets < 3)
            count++;

        if (effStack * 2 - (7 * toCall) > 2 * currCommited && numBets < 3)
            count++;

        if (effStack > prevBet)
            count++;
    }
    return count;
}

int Board::getStreetIndex() const {
    return boardIndices[street-1];
}

int Board::getPrivateHandIndex() const {
    return (currentPlayer == hero ? heroIndices[street] : villainIndices[street]);
}

int Board::getPrivateKeyCount() const {
    if (street == 0) {
        return 169;
    } else if (street == 1){
        return 1176;
    } else if (street == 2){
        return 1128;
    } else if (street == 3){
        return 1081;
    } else {
        cout << "BUG HERE, CALL PRIVATE KEY COUNT ON 4'th STREET";
    }
}

bool Board::switchedStreets() const {
    return streetChange;
}

int Board::getStreetOutcomes() const {
    if (street == 0){
        return 169;
    } else {
        return (50 - street + 1); //50 buckets, 49 turn, 48 river cards
    }
}

