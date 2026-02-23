#pragma once 

#include <cstdint>

enum NodeType : uint8_t {
    ACTION_NODE = 0,
    CHANCE_NODE = 1,
    TERMINAL_NODE = 2
};