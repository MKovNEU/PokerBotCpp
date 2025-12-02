#pragma once 
#include <string>

enum Ranks {
    TWO,
    THREE,
    FOUR, 
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE
};

inline std::string rankString(Ranks r) {
    switch (r) {
        case Ranks::TWO:   return "2";
        case Ranks::THREE: return "3";
        case Ranks::FOUR:  return "4";
        case Ranks::FIVE:  return "5";
        case Ranks::SIX:   return "6";
        case Ranks::SEVEN: return "7";
        case Ranks::EIGHT: return "8";
        case Ranks::NINE:  return "9";
        case Ranks::TEN:   return "T";
        case Ranks::JACK:  return "J";
        case Ranks::QUEEN: return "Q";
        case Ranks::KING:  return "K";
        case Ranks::ACE:   return "A";
        default: return "U";
    }
}