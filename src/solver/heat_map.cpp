
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>

using namespace std;

string ranks = "AKQJT98765432";

// Helper to get the rank character from a string like "3♣" or "A♦"
char getRank(const string& cardStr) {
    if (cardStr.empty()) return '?';
    // Handles 10 as 'T' if your decode does that, or just returns the first char
    return cardStr[0]; 
}

// Helper to get the suit part to compare for "suitedness"
string getSuit(const string& cardStr) {
    if (cardStr.size() < 2) return "";
    return cardStr.substr(1); // Returns the symbol like "♣"
}

void generateHeatmap(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file!" << endl;
        return;
    }

    string line;
    map<string, pair<float, int>> handMap;

    while (getline(file, line)) {
        // We only want preflop: exactly 2 cards and no moves after the "|"
        // Example: Cards: A♣ K♣ | Moves: 0.5 0.5
        if (line.find("Moves: A F F") == string::npos) continue;

        stringstream ss(line);
        string temp, c1Str, c2Str, bar;
        
        ss >> temp; // "Cards:"
        ss >> c1Str; // "A♣"
        ss >> c2Str; // "K♣"
        ss >> bar;   // "|"
        ss >> temp;  // "Moves:"
        ss >> temp;
        ss >> temp;
        ss >> temp;

        // Extract probabilities - we'll take the very last number as the "Jam" prob
        float jamProb = 0.0f;
        vector<float> probs;
        float p;
        while (ss >> p) {
            probs.push_back(p);
            ss >> temp;
        }
        if (probs.empty()) continue;
        jamProb = probs.back(); // Assuming All-In is the last action

        // Identify the hand
        char r1 = getRank(c1Str);
        char r2 = getRank(c2Str);
        bool suited = (getSuit(c1Str) == getSuit(c2Str));

        string key;
        if (r1 == r2) {
            key = string(1, r1) + r1;
        } else {
            // Sort ranks by strength
            if (ranks.find(r1) < ranks.find(r2)) key = string(1, r1) + r2;
            else key = string(1, r2) + r1;
            key += (suited ? "s" : "o");
        }

        handMap[key].first += jamProb;
        handMap[key].second += 1;
    }

    // --- PRINTING LOGIC ---
    cout << "\n--- UTG OPENING HEATMAP (Last Action Frequency) ---\n\n     ";
    for(char r : ranks) cout << r << "     ";
    cout << "\n";

    for (int i = 0; i < 13; i++) {
        cout << ranks[i] << " ";
        for (int j = 0; j < 13; j++) {
            string key;
            if (i == j) key = string(1, ranks[i]) + ranks[j];
            else if (i < j) key = string(1, ranks[i]) + ranks[j] + "s";
            else key = string(1, ranks[j]) + ranks[i] + "o";

            float val = (handMap[key].second > 0) ? (handMap[key].first / handMap[key].second) : 0;
            cout << fixed << setprecision(2) << "[" << val << "]";
        }
        cout << "\n";
    }
}

int main() {
    generateHeatmap("output.txt");
    return 0;
}