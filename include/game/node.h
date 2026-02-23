#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include "include/game/card.h"
#include "include/solver/infoset.h"
#include "include/enums/moves.h"
#include "include/game/undo_info.h"
#include "include/enums/players.h"
#include "include/solver/buckets.hpp"
#include "include/game/board.h"
#include "include/enums/node_type.h"
#include "include/enums/streets.h"

using namespace std;

struct PublicNode {
    NodeType type;
    vector<float> regrets;
    vector<float> strategySum;

    vector<PublicNode*> children;
    
    PublicNode(NodeType t, int num_buckets, int num_actions) : type(t) { //action node
        regrets.assign(num_buckets * num_actions, 0.0f);
        strategySum.assign(num_buckets * num_actions, 0.0f);
        children.assign(num_actions, nullptr);
    }

    PublicNode(NodeType t, int numOutcomes) : type(t) { //chance node
        children.assign(numOutcomes, nullptr);
    }

    PublicNode(NodeType t) : type(t) {} //terminal node
};

