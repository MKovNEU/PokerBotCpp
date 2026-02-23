#pragma once 
#include <string>

enum Suits {
    CLUBS,
    DIAMONDS,
    HEARTS,
    SPADES
};

inline std::string suitString(Suits s) {
    switch (s) {
        case Suits::CLUBS:   return "♣";
        case Suits::DIAMONDS: return "♦";
        case Suits::HEARTS:    return "♥";
        case Suits::SPADES:   return "♠";
        default: return "U";
    }
}