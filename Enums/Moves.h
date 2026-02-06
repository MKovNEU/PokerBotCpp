#pragma once 
#include <string>

enum Moves {
    NULL_MOVE, //placeholder for start of street. 
    FOLD, //fold
    ALL_IN // Go  All-in
};

inline std::string moveString(Moves m) {
    switch (m) {
        case FOLD:      return "F";
        case ALL_IN:    return "A";
        case NULL_MOVE: return "N";
        default:        return "U";
    }
}

