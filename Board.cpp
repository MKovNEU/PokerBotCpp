#include <iostream>
#include "Board.h"

void Board::move(Moves move) {
    if (move == FOLD) {
        bool anyAllIn = false;
        //check if UTG or BUTTON whent all in
        for (int i = 0; i < 2; i++) {
            anyAllIn = anyAllIn || allIn[i];
        }
        if (currentPlayer == BB || (currentPlayer == SB && !anyAllIn)) terminal = true;
    } else {
        allIn[currentPlayer] = true;
        if (currentPlayer == BB) terminal = true;
    }
    if (!terminal) {
        currentPlayer = (Players)(currentPlayer + 1);
        for (int p = 0; p < 4; p++) {
            infosets[p].addMove(move);
        }
    }
}

double Board::utilities() {
    vector<int> jammers;
    for (int i = 0; i < 4; i++) {
        if (allIn[i]) jammers.push_back(i);
    }

    if (jammers.size() == 0) { //folded to BB
        if (hero == BB) return 0.5; 
        if (hero == SB) return -0.5;
        return 0.0; 
    }
    
    if (!allIn[hero]) { //someone jammed, hero folded
        if (hero == SB) return -0.5;
        if (hero == BB) return -1.0;
        return 0.0;
    }
    /*
    if (jammers.size() == 1) { //hero jammed, won no contest
        double profit = 0;
        if (hero != SB) profit += 0.5;
        if (hero != BB) profit += 1.0;
        return profit;
    }
    */
    vector<pair<int, double>> bets; //position in table, bet
    for (int i = 0; i < 4; i++) {
        if (allIn[i]) {
            bets.push_back(make_pair(i, stacks[i]));
        }
    }
    sort(bets.begin(), bets.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
        return a.second < b.second;
    });
    vector<double> newStacks(4, 0.0);
    vector<pair<double, vector<int>>> sidePots; //side pots, with a pair of total pot size and the players in pot
    for (int i = 0; i < bets.size(); i++) {
        if (bets[i].second <= 0) continue;
        double take = bets[i].second;
        vector<int> pSidePot; //players in this side pot
        double totalAmnt = (i == 0) ? potSize : 0;
        for (int j = i; j < bets.size(); j++) {
            bets[j].second -= take;
            totalAmnt += take;
            pSidePot.push_back(bets[j].first);
        }
        sidePots.push_back(make_pair(totalAmnt, pSidePot));
    }
    for (const auto &pot : sidePots) {
        vector<pair<int, int>> scores; // position in table, score;
        for (int player : pot.second) {
            scores.push_back(make_pair(player, handScores[player])); //smaller scores better
        }
        sort(scores.begin(), scores.end(), [](const pair<int, int> &a, const pair<int, int> &b) {
            return a.second > b.second;
        });
        int same = 1;
        while(same < pot.second.size() && scores[same - 1].second == scores[same].second) {
            same++;
        }
        for (int i = 0; i < same; i++) {
            newStacks[scores[i].first] += (pot.first / same);
        }
    }
    double startingStack = stacks[hero] + ((hero == SB) ? 0.5 : 0) + ((hero == BB) ? 1 : 0);
    //std::cout << cards[hero][0].toString() << " " << cards[hero][1].toString() << " " << (newStacks[hero] - startingStack) << "\n";
    if (cards[hero][0].rank == ACE && cards[hero][0].rank == ACE) {
        acesWin += (newStacks[hero] - startingStack);
        //cout << acesWin << "\n"; 
    }
    return (newStacks[hero] - startingStack);
}

vector<Moves> Board::getLegalActions() const {
    vector<Moves> actions;
    actions.push_back(FOLD);
    actions.push_back(ALL_IN);
    return actions;
}

int Board::getNumActions() const {
    return getLegalActions().size();
}

