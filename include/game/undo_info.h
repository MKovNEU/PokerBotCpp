#pragma once

#include "include/enums/moves.h"
#include "include/enums/players.h"
#include "include/solver/infoset.h"

struct UndoInfo {

    Moves lastMove;
    bool terminal;
    bool streetChange;
    bool firstMove;
    Players currentPlayer;
    int potSize;
    int prevBet;
    int numBets;
    int street;


    int heroStack;
    int villainStack;
    int heroCommittedStreet;
    int villainCommittedStreet;
    int heroCommittedTotal;
    int villainCommittedTotal;
};