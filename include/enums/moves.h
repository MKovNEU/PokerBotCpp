#pragma once 
#include <string>

enum Moves {
    NULL_MOVE, //placeholder for start of street. 
    FOLD, //fold
    CALL, //check and call
    RAISE1, //Bet 40% of the pot, or 2.5x prev bet
    RAISE2, //bet 80% of the pot, or 3.5x prev bet
    ALL_IN //all in
};

inline std::string moveString(Moves m) {
    switch (m) {
        case FOLD:      return "F";
        case CALL:      return "C";
        case RAISE1:    return "R1";
        case RAISE2:    return "R2";
        case ALL_IN:    return "A";
        case NULL_MOVE: return "N";
        default:        return "U";
    }
}

