#include <iostream>
#include <vector>
#include <set>
#include <array>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <random>
#include <cmath>
#include "include/game/card.h"
#include "include/enums/suits.h"
#include "include/enums/ranks.h"
#include "include/game/hand_evaluator.h"

using namespace std;


struct ArrayCardHash {
    size_t operator()(const std::array<Card, 3>& v) const {
        size_t seed = 0;
        // You can unroll this loop manually since size is fixed
        size_t h1 = (size_t(v[0].rank) << 2) | size_t(v[0].suit);
        size_t h2 = (size_t(v[1].rank) << 2) | size_t(v[1].suit);
        size_t h3 = (size_t(v[2].rank) << 2) | size_t(v[2].suit);
        
        // Manual combine
        seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

unordered_map<array<Card, 3>, int, ArrayCardHash> flopBuckets;

long long factorial(int n) {
    long long res = 1;
    for (int i = 2; i <= n; i++)
        res *= i;
    return res;
}

// Function to calculate nCr
long long nCr(int n, int r) {
    if (r > n || r < 0) return 0; // Invalid combination
    if (r == 0 || r == n) return 1;
    // Use symmetry C(n, r) = C(n, n-r) to reduce calculations
    if (r > n / 2) r = n - r; 
    return factorial(n) / (factorial(r) * factorial(n - r));
}

string encodeHandBoard(const vector<Card> &hand, const vector<Card> &board);

// Helper: Convert string "Q♥" -> Card Object
Card stringToCard(string s) {
    // 1. Extract Rank
    char rChar = s[0];
    Ranks r;
    switch (rChar) {
        case '2': r = TWO; break;
        case '3': r = THREE; break;
        case '4': r = FOUR; break;
        case '5': r = FIVE; break;
        case '6': r = SIX; break;
        case '7': r = SEVEN; break;
        case '8': r = EIGHT; break;
        case '9': r = NINE; break;
        case 'T': r = TEN; break;
        case 'J': r = JACK; break;
        case 'Q': r = QUEEN; break;
        case 'K': r = KING; break;
        case 'A': r = ACE; break;
        default: r = TWO; // Should not happen
    }

    // 2. Extract Suit (Handle UTF-8 or ASCII)
    // The suit starts at s[1]. Note: UTF-8 hearts/spades are 3 bytes long.
    // Simple check: look at the last char or substring match
    Suits suit;
    if (s.find("♣") != string::npos || s.find("c") != string::npos) suit = CLUBS;
    else if (s.find("♦") != string::npos || s.find("d") != string::npos) suit = DIAMONDS;
    else if (s.find("♥") != string::npos || s.find("h") != string::npos) suit = HEARTS;
    else if (s.find("♠") != string::npos || s.find("s") != string::npos) suit = SPADES;
    else suit = CLUBS; // Default error case

    return Card(r, suit);
}

// MAIN FUNCTION: Load from File
vector<pair<vector<Card>, vector<float>>> loadHistograms(string filename) {
    vector<pair<vector<Card>, vector<float>>> data;
    ifstream infile(filename);
    
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return data;
    }

    string line;
    while (getline(infile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string temp, c1, c2, c3;
        
        // Expected Format:
        // Flop: 2♥ 2♦ 2♣ | Equity Histogram: 0.0 ...
        
        ss >> temp; // "Flop:"
        if (temp != "Flop:") continue; // Skip malformed lines

        ss >> c1 >> c2 >> c3; // "2♥", "2♦", "2♣"
        
        // Read the separator and header parts
        // " | Equity Histogram: " -> verify or skip
        // This loop eats words until it sees a number or runs out
        // The format has " | ", "Equity", "Histogram:"
        ss >> temp; // "|"
        ss >> temp; // "Equity"
        ss >> temp; // "Histogram:"

        // Parse Cards
        vector<Card> flop;
        flop.push_back(stringToCard(c1));
        flop.push_back(stringToCard(c2));
        flop.push_back(stringToCard(c3));

        // Parse Histogram (50 floats)
        vector<float> histogram;
        float val;
        while (ss >> val) {
            histogram.push_back(val);
        }

        // Sanity check
        if (histogram.size() == 50) {
            data.push_back({flop, histogram});
        } else {
            cerr << "Warning: Line had " << histogram.size() << " bins instead of 50." << endl;
        }
    }
    
    cout << "Successfully loaded " << data.size() << " histograms." << endl;
    return data;
}

int isomorph(const vector<Card> &hand) {
    int r1 = (int)hand[0].rank;
    int r2 = (int)hand[1].rank;
    
    // 1. Always ensure r1 is the higher rank (e.g., Ace > King)
    if (r1 < r2) std::swap(r1, r2);

    // 2. Case: Pocket Pairs (Indices 0 - 12)
    if (r1 == r2) {
        return r1; 
    }

    // 3. Case: Suited Hands (Indices 13 - 90)
    if (hand[0].suit == hand[1].suit) {
        // We use the triangular number formula for the 78 suited combos
        // (r1 * (r1 - 1) / 2) creates the unique offset for the high card
        return 13 + (r1 * (r1 - 1) / 2) + r2;
    } 
    
    // 4. Case: Offsuit Hands (Indices 91 - 168)
    else {
        return 91 + (r1 * (r1 - 1) / 2) + r2;
    }
}

#include <string>
#include <vector>

std::string indexToHandString(int index) {
    if (index < 0 || index > 168) return "Invalid Index";

    // Rank strings for easy mapping
    const char rankChars[] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};

    // 1. Case: Pocket Pairs (0 - 12)
    if (index <= 12) {
        char r = rankChars[index];
        // Using two different suits for a pair (Clubs and Spades)
        return std::string(1, r) + "c" + r + "s";
    }

    // 2. Case: Suited Hands (13 - 90)
    if (index <= 90) {
        int adjusted = index - 13;
        // Solve (r1 * (r1 - 1) / 2) + r2 = adjusted
        for (int r1 = 1; r1 < 13; ++r1) {
            for (int r2 = 0; r2 < r1; ++r2) {
                if ((r1 * (r1 - 1) / 2) + r2 == adjusted) {
                    return std::string(1, rankChars[r1]) + "s" + rankChars[r2] + "s";
                }
            }
        }
    }

    // 3. Case: Offsuit Hands (91 - 168)
    if (index <= 168) {
        int adjusted = index - 91;
        for (int r1 = 1; r1 < 13; ++r1) {
            for (int r2 = 0; r2 < r1; ++r2) {
                if ((r1 * (r1 - 1) / 2) + r2 == adjusted) {
                    // Using Diamonds and Clubs to represent offsuit
                    return std::string(1, rankChars[r1]) + "d" + rankChars[r2] + "c";
                }
            }
        }
    }

    return "Error";
}


vector<vector<Card>> isomorph(const vector<Card> &hand, const vector<Card> &flop){
    vector<Card> canonHand;
    vector<Card> canonFlop;
    vector<Card> sHand = hand;
    vector<Card> sFlop= flop;

    auto rankSort = [](const Card &a, const Card &b) {
        return a.encode() > b.encode();
    };

    sort(sHand.begin(), sHand.end(), rankSort);
    sort(sFlop.begin(), sFlop.end(), rankSort);
    

    int suitMap[4] = {-1, -1, -1, -1};
    //cout << suitMap[0] << suitMap[1] << suitMap[2] << suitMap[3] << "\n";
    Suits nextSuit = CLUBS;

    pair<int, int> flopCount[4] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}; 
    pair<int, int> handCount[4] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    
    for (const Card &card : sFlop) {
        flopCount[card.suit].first++;
        flopCount[card.suit].second = max(flopCount[card.suit].second, int(card.rank));
    }
    
    for (const Card &card : sHand) {
        handCount[card.suit].first++;
        handCount[card.suit].second = max(handCount[card.suit].second, int(card.rank));
    }

    // Build suit counts for unmapped suits, in flop order
    struct suitConfig {
        int suit;
        int flopCount;
        int handCount;
        int flopRank; //highest rank of card in flop with this suit, for tie-breaking
        int handRank; // highest rank of card in hand with this suit, for tie-breaking
    };

    vector<suitConfig> suitConfigs;

    for (int s = 0; s < 4; s++) {
        suitConfigs.push_back({s, flopCount[s].first, handCount[s].first, flopCount[s].second, handCount[s].second});
    }


    
    stable_sort(suitConfigs.begin(), suitConfigs.end(), [](const suitConfig &a, const suitConfig &b) {
        if (a.flopCount != b.flopCount) {
            return a.flopCount > b.flopCount; // More cards in flop first
        }
        if (a.flopRank != b.flopRank) {
            return a.flopRank > b.flopRank; //Tie-break with highest rank in flop
        } 
        if (a.handCount != b.handCount) {
            return a.handCount > b.handCount; // Then more cards in hand
        }
        return a.handRank > b.handRank; // Tie-break with highest rank in hand
    });
    /*
    cout << "suitCounts: ";
    for (auto &sc : suitCounts) {
        cout << "(" << sc.first << "," << sc.second << ") ";
    }
    cout << endl;
    */
    // Map remaining suits
    for (const auto &sc : suitConfigs) {
        suitMap[sc.suit] = nextSuit;
        nextSuit = Suits(nextSuit + 1);
    }
    
    // Map board
    for (const Card &card : sFlop) {
        canonFlop.push_back(Card(card.rank, static_cast<Suits>(suitMap[card.suit])));
    }

    for (const Card &card : sHand) {
        if (suitMap[card.suit] == -1) {
            suitMap[card.suit] = nextSuit;
            nextSuit = Suits(nextSuit + 1);
        }
        canonHand.push_back(Card(card.rank, static_cast<Suits>(suitMap[card.suit])));
    }

    sort(canonHand.begin(), canonHand.end(), rankSort);
    sort(canonFlop.begin(), canonFlop.end(), rankSort);
    
    return {canonHand, canonFlop};
}

// Encode a hand+board into a unique string for comparison
string encodeHandBoard(const vector<Card> &hand, const vector<Card> &board) {
    string result;
    for (const Card &c : hand) {
        result += c.toString();
    }
    result += "|";
    for (const Card &c : board) {
        result += c.toString();
    }
    return result;
}

string encodePreflopBoard(const vector<Card> &hand) {
    string result;
    for (const Card &c : hand) {
        result += c.toString();
    }
    return result;
}

set<pair<vector<Card>, vector<Card>>> testAllIsomorphisms() {
    vector<Card> deck;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck.push_back(Card(Ranks(r), Suits(s)));
        }
    }
    
    set<string> uniqueIsomorphicClasses;
    set<pair<vector<Card>, vector<Card>>> uniqueCanonicalForms;
    long long totalCombinations = 0;
    
    cout << "Testing all hand + flop combinations...\n";
    cout << "This will take a moment...\n\n";
    
    // Iterate through all possible 2-card hands + 3-card flops
    for (int h1 = 0; h1 < 52; h1++) {
        for (int h2 = h1 + 1; h2 < 52; h2++) {
            for (int f1 = 0; f1 < 52; f1++) {
                if (f1 == h1 || f1 == h2) continue;
                for (int f2 = f1 + 1; f2 < 52; f2++) {
                    if (f2 == h1 || f2 == h2) continue;
                    for (int f3 = f2 + 1; f3 < 52; f3++) {
                        if (f3 == h1 || f3 == h2) continue;
                        vector<Card> hand = {deck[h1], deck[h2]};
                        vector<Card> board = {deck[f1], deck[f2], deck[f3]};
                        
                        // Get isomorphic representation
                        auto iso = isomorph(hand, board);
                        uniqueCanonicalForms.insert({iso[1], iso[0]});
                        string canonical = encodeHandBoard(iso[0], iso[1]);
                        uniqueIsomorphicClasses.insert(canonical);

                        totalCombinations++;
                    }
                }
            }
        }
        
        // Progress indicator
        if (h1 % 10 == 0) {
            cout << "Progress: " << h1 << "/52 hands processed...\r" << flush;
        }
    }

    cout << "Unique isomorphic classes: " << uniqueIsomorphicClasses.size() << "\n";

    return uniqueCanonicalForms;
}

/*
set<vector<Card>> testPreflopIso() {
    vector<Card> deck;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck.push_back(Card(Ranks(r), Suits(s)));
        }
    }
    
    set<pair<int, string>> uniquePreflopIsomorphicClasses;
    set<vector<Card>> uniqueCanonicalForms;
    long long totalCombinations = 0;
    
    cout << "Testing all hand combinations...\n";
    
    // Iterate through all possible 2-card hands + 3-card flops
    for (int h1 = 0; h1 < 52; h1++) {
        for (int h2 = h1 + 1; h2 < 52; h2++) {
            vector<Card> hand = {deck[h1], deck[h2]};
                        
             // Get isomorphic representation
            auto iso = isomorph(hand);
            string canonical = encodePreflopBoard(iso.second);
            uniqueCanonicalForms.insert(iso.second);
            uniquePreflopIsomorphicClasses.insert(make_pair(iso.first, canonical));

            totalCombinations++;
        }
    }
    
    cout << "Unique isomorphic classes: " << uniquePreflopIsomorphicClasses.size() << "\n";

    return uniqueCanonicalForms;
}
*/

void testIsomorphism() {
vector<Card> h1 = {Card(ACE, DIAMONDS), Card(ACE, SPADES)};
vector<Card> b1 = {Card(KING, DIAMONDS), Card(JACK, HEARTS), Card(NINE, SPADES)};

vector<Card> h2 = {Card(ACE, DIAMONDS), Card(ACE, SPADES)};  
vector<Card> b2 = {Card(KING, SPADES), Card(JACK, CLUBS), Card(NINE, DIAMONDS)};

// Both: same hand, flop with 2 of one suit + 1 of another
// Difference: which rank has the singleton suit

    // Should also be identical

    auto res1 = isomorph(h1, b1);
    auto res2 = isomorph(h2, b2);
    string can1 = encodeHandBoard(res1[0], res1[1]);
    string can2 = encodeHandBoard(res2[0], res2[1]);
    cout << can1 << " " << can2;
}

bool sharedCards(const vector<Card> &hand, const vector<Card> &board) {
    for (const Card &c1 : hand) {
        for (const Card &c2 : board) {
            if (c1 == c2) return true;
        }
    }
    return false;
}

std::mt19937 rng(std::random_device{}());

// Optimized MCEquity
float calculateMCEquity(const vector<Card> &hand, const vector<Card> &flop, const vector<Card> &fullDeck, int iterations) {
    // 1. Build remaining deck ONCE
    vector<Card> remainingDeck;
    remainingDeck.reserve(47);
    for (const Card &c : fullDeck) {
        if (!sharedCards(hand, {c}) && !sharedCards(flop, {c})) {
            remainingDeck.push_back(c);
        }
    }

    int wins = 0;
    int ties = 0;
    
    for (int i = 0; i < iterations; i++) {
        for (int k = 0; k < 4; k++) {
            std::uniform_int_distribution<int> dist(k, remainingDeck.size() - 1);
            int swapIdx = dist(rng);
            std::swap(remainingDeck[k], remainingDeck[swapIdx]);
        }

        vector<Card> oppHand = {remainingDeck[0], remainingDeck[1]};
        vector<Card> board = {flop[0], flop[1], flop[2], remainingDeck[2], remainingDeck[3]};

        int pScore = evaluateHand(hand, board);
        int oScore = evaluateHand(oppHand, board);

        if (pScore > oScore) wins++;
        else if (pScore == oScore) ties++;
    }
    return float(wins + (ties * 0.5f)) / iterations;
}

vector<pair<vector<Card>, vector<float>>> generateVectors(set<pair<vector<Card>, vector<Card>>> canonForms) {
    vector<Card> deck;
    for (int s = CLUBS; s <= SPADES; s++) {
        for (int r = TWO; r <= ACE; r++) {
            deck.push_back(Card(Ranks(r), Suits(s)));
        }
    }

    vector<pair<vector<Card>, vector<vector<Card>>>> flopGroups; 
    for (const auto& p : canonForms) {
        const vector<Card> &hand = p.second;
        const vector<Card> &flop = p.first;

        if (flopGroups.empty() || flopGroups.back().first != flop) {
            flopGroups.push_back({flop, {hand}});
        } else {
            flopGroups.back().second.push_back(hand);
        }
    }

    for (const auto &group : flopGroups) {
        cout << "Flop: ";
        for (const Card &c : group.first) {cout << c.toString() << " ";}
        cout << " | Preflop Hands: " << group.second.size() << "\n";
    }
    cout << "Total unique flops: " << flopGroups.size() << endl;

    vector<pair<vector<Card>, vector<float>>> flopEquityHistogram; // For each flop,
    //histogram of equities of all hands in that flop group. 0-2%, 3-4%, ..., 98-100%
    int histogramBins = 50;
    int mcIterations = 2500;

    for (const auto &group : flopGroups) {
        const vector<Card> &flop = group.first;
        vector<float> histogram(histogramBins, 0.0f);
        int validHandCount = 0;

        for (const auto &hand : group.second) {
            float equity = calculateMCEquity(hand, flop, deck, mcIterations);

            int bin = min(int(equity * histogramBins), histogramBins - 1);
            histogram[bin] += 1.0f;
            validHandCount++;
        }

        for (int i = 0; i < histogramBins; i++) {
            if (validHandCount > 0) histogram[i] /= validHandCount;
        }

        flopEquityHistogram.push_back({flop, histogram});

        if (flopEquityHistogram.size() % 100 == 0) {
            cout << "Processed " << flopEquityHistogram.size() << " flops..." << endl;
        }
    }

    ofstream file("bucketsOut2.txt");
    for (const auto &entry : flopEquityHistogram) {
        const vector<Card> &flop = entry.first;
        const vector<float> &histogram = entry.second;
        
        file << "Flop: ";
        for (const Card &c : flop) {file << c.toString() << " ";}
        file << " | Equity Histogram: ";
        for (float val : histogram) {
            file << val << " ";
        }
        file << "\n";
    }
    return flopEquityHistogram;
}

float getDistance(const vector<float> &cdf1, const vector<float> &cdf2) {
    float dist = 0.0f;
    for (int i = 0; i < cdf1.size(); i++) {
        dist += abs(cdf1[i] - cdf2[i]);
    }
    return dist;
}

vector<vector<float>> init_k_means_plus_plus(const vector<pair<array<Card, 3>, vector<float>>> &cdfHistograms, int k) {
    vector<vector<float>> centroids;
    int n = cdfHistograms.size();
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, n - 1);

    // Select the first centroid randomly
    centroids.push_back(cdfHistograms[dis(gen)].second);

    // Select remaining k-1 centroids
    for (int c = 1; c < k; ++c) {
        vector<float> distances(n, numeric_limits<float>::max());

        // Calculate distance to nearest existing centroid
        for (int i = 0; i < n; ++i) {
            for (const auto &centroid : centroids) {
                float dist = getDistance(cdfHistograms[i].second, centroid);
                distances[i] = min(distances[i], dist);
            }
        }

        // Select next centroid with probability proportional to distance squared
        float sumDistances = 0.0f;
        for (float d : distances) {
            sumDistances += d * d;
        }

        uniform_real_distribution<> disReal(0, sumDistances);
        float r = disReal(gen);
        float cumulative = 0.0f;
        for (int i = 0; i < n; ++i) {
            cumulative += distances[i] * distances[i];
            if (cumulative >= r) {
                centroids.push_back(cdfHistograms[i].second);
                break;
            }
        }
    }

    return centroids;
}

pair<unordered_map<array<Card, 3>, int, ArrayCardHash>, unordered_map<int, vector<array<Card, 3>>>> k_means_clustering(const vector<pair<vector<Card>, vector<float>>> &flopHistograms, int k) {    
    unordered_map<array<Card, 3>, int, ArrayCardHash> flopBuckets;
    unordered_map<int, vector<array<Card, 3>>> bucketFlop; 
    vector<pair<array<Card, 3>, vector<float>>> cdfHistograms;
    
    // convert histograms to CDFs for fast EMD calculation
    for (const auto &entry : flopHistograms) {
        const vector<Card> &flop = entry.first;
        const vector<float> &histogram = entry.second;
        vector<float> cdf(histogram.size(), 0.0f);
        float sum = 0.0f;
        for (size_t i = 0; i < histogram.size() - 4; ++i) {
            sum += histogram[i];
            cdf[i] = sum;
        }
        for (size_t i = histogram.size() - 4; i < histogram.size(); ++i) {
            cdf[i] = histogram[i];
        }
        array<Card, 3> flopArr = {flop[0], flop[1], flop[2]};
        cdfHistograms.push_back({flopArr, cdf});
    }

    vector<vector<float>> centoids = init_k_means_plus_plus(cdfHistograms, k);

    int iter = 0;
    bool changed = true;
    while(changed) {
        changed = false;
        for (int i = 0; i < cdfHistograms.size(); ++i) {
            float minDist = numeric_limits<float>::max();
            int bestCluster = -1;
            for (int c = 0; c < k; ++c) {
                float dist = getDistance(cdfHistograms[i].second, centoids[c]);
                if (dist < minDist) {
                    minDist = dist;
                    bestCluster = c;
                }
            }
            flopBuckets[cdfHistograms[i].first] = bestCluster;
            bucketFlop[bestCluster].push_back(cdfHistograms[i].first);
        }

        for (int c = 0; c < k; c++) {
            vector<float> newCentroid = vector<float>(cdfHistograms[0].second.size(), 0.0f);
            for (const auto &p : cdfHistograms) {
                if (flopBuckets[p.first] != c) continue;
                for (int i = 0; i < newCentroid.size(); i++) {
                    newCentroid[i] += p.second[i];
                }
            }
            for (int i = 0; i < newCentroid.size(); i++) {
                newCentroid[i] /= bucketFlop[c].size();
            }
            if (getDistance(newCentroid, centoids[c]) > 1e-6) {
                changed = true;
                centoids[c] = newCentroid;
            }
        }

        if (changed) {
            flopBuckets.clear();
            bucketFlop.clear();
        }

        cout << "K-Means Iteration " << iter++ << " completed." << endl;
    }

    return make_pair(flopBuckets, bucketFlop);
}

void printBuckets(const unordered_map<int, vector<array<Card, 3>>> &bucketFlop) {
    ofstream file("bucketsFinal3.txt");
    for (const auto &entry : bucketFlop) {
        file << "Bucket " << entry.first << " has " << entry.second.size() << " flops." << endl;
        for (const auto &flop : entry.second) {
            file << "  Flop: ";
            for (const Card &c : flop) {
                file << c.toString() << " ";
            }
            file << endl;
        }
    }
}

void addFeatures(vector<pair<vector<Card>, vector<float>>> &flopHistograms) {
    // 1. Define Weights
    // We make these floats so the math works smoothly
    const float W_RANK   = 2.0f; // High Card
    const float W_STR    = 0.7f; // Connectivity
    const float W_FLSH   = 0.7f; // Suitedness
    const float W_PAIR   = 1.0f; // Pairedness

    for (auto &entry : flopHistograms) {
        const vector<Card> &flop = entry.first;
        vector<float> &histogram = entry.second;

        int maxRank = max({flop[0].rank, flop[1].rank, flop[2].rank});
        float featHighCard = (float(maxRank) / 12.0f); 

        vector<int> ranks = {flop[0].rank, flop[1].rank, flop[2].rank};
        std::sort(ranks.begin(), ranks.end());
        
        float featStraight = 0.0f;
        
        bool isWheel = (ranks[2] == 12 && ranks[0] <= 3 && ranks[1] <= 4);

        if (ranks[2] - ranks[0] <= 4 || isWheel) {
            if ((!isWheel && ranks[2] - ranks[0] <= 2) || (isWheel && ranks[1] - ranks[0] <= 2)) {
                featStraight = 1.0f; 
            } else {
                featStraight = 0.5f;
            }
        }


        float featFlush = 0.0f;
        if (flop[0].suit == flop[1].suit && flop[0].suit == flop[2].suit) {
            featFlush = 1.0f; // Monotone
        } else if (flop[0].suit == flop[1].suit || flop[0].suit == flop[2].suit || flop[1].suit == flop[2].suit) {
            featFlush = 0.5f; // Two-Tone
        }


        float featPaired = 0.0f;
        if (flop[0].rank == flop[1].rank && flop[0].rank == flop[2].rank) {
            featPaired = 1.0f; // Trips
        } else if (flop[0].rank == flop[1].rank || flop[0].rank == flop[2].rank || flop[1].rank == flop[2].rank) {
            featPaired = 0.5f; // Paired
        }

        histogram.push_back(featHighCard * W_RANK);
        histogram.push_back(featStraight * W_STR);
        histogram.push_back(featFlush    * W_FLSH);
        histogram.push_back(featPaired   * W_PAIR);
    }
}


void generateBuckets() {
    //loadHandRanks("HandRanks.dat");
    //set<pair<vector<Card>, vector<Card>>> canonForms = testAllIsomorphisms();
    //generateVectors(canonForms);
    auto loadedData = loadHistograms("data/equity_histograms.txt");
    cout << "Loaded data size: " << loadedData.size() << endl;
    addFeatures(loadedData);
    auto result = k_means_clustering(loadedData, 50);
    flopBuckets = result.first;

    //unordered_map<int, vector<array<Card, 3>>> bucketFlop = result.second;
}

int getBucket(const vector<Card> &flop) {
    array<Card, 3> flopArr = {flop[0], flop[1], flop[2]};
    if (flopBuckets.find(flopArr) != flopBuckets.end()) {
        return flopBuckets[flopArr];
    } else {
        cerr << "Error: Flop not found in buckets!" << endl;
        return -1; // Or some error code
    }
}

int rawIndex(const std::vector<Card>& hand, const std::vector<Card>& board) {
    vector<int> boardValues;
    for (const Card &c : board) {
        boardValues.push_back(c.encode() - 1);
    }

    int c1 = hand[0].encode() - 1;
    int c2 = hand[1].encode() - 1;

    auto compress = [&](int card) {
        int adjusted = card;
        for (int val : boardValues) {
            if (card > val) adjusted--;
        }
        return adjusted;
    };

    int h1 = compress(c1);
    int h2 = compress(c2);

    if (h1 > h2) {
        std::swap(h1, h2);
    }

    return (h2 * (h2 - 1) / 2) + h1;
}

int rawIndex(const Card &hand, const std::vector<Card>& board) {
    vector<int> boardValues;
    for (const Card &c : board) {
        boardValues.push_back(c.encode() - 1);
    }

    int c1 = hand.encode() - 1;

    auto compress = [&](int card) {
        int adjusted = card;
        for (int val : boardValues) {
            if (card > val) adjusted--;
        }
        return adjusted;
    };

    int h1 = compress(c1);

    return h1;
}

