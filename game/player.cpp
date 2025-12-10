#include "player.h"
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <limits>
#include <map>

namespace {
    std::string comboToString(const std::vector<Card>& cards) {
        if (cards.empty()) return "PASS";
        std::string s;
        for (size_t i = 0; i < cards.size(); ++i) {
            if (i) s += ' ';
            s += cards[i].CardToString();
        }
        return s;
    }

    std::string handTypetoString(HandType type) {
        switch (type) {
            case HandType::Single:          return "Single";
            case HandType::Pair:            return "Pair";
            case HandType::Straight:        return "Straight";
            case HandType::FullHouse:       return "Full House";
            case HandType::FourOfAKind:     return "Four of a Kind";
            case HandType::StraightFlush:   return "Straight Flush";
            default:                        return "Invalid";
        }
    }
}

/* Player Baseline */

Player::Player(int seatIndex, int DB_id) : seatIndex(seatIndex), DB_id(DB_id), isActive(true) {}

int Player::getseatIndex() const { return seatIndex; }
int Player::getDBId() const { return DB_id; }
void Player::setDBId(int new_DB_id) { DB_id = new_DB_id; }

Hand& Player::getHand() { return hand; }
const Hand& Player::getHand() const { return hand; }
bool Player::isPlayerActive() const { return isActive; }

void Player::deactivate() { isActive = false; }
void Player::addCard(const Card& card) { hand.addCard(card); }

/* HumanPlayer Implementation */

HumanPlayer::HumanPlayer(int seatIndex, int DB_id) : Player(seatIndex, DB_id) {}

std::vector<Card> HumanPlayer::playTurn(const Combination& lastPlay, const std::vector<std::vector<Card>>& legalActions, bool canPass) {
    (void)lastPlay; // Unused parameter for now
    std::cout << "Seat " << getseatIndex() + 1 << "'s turn.\n";
    std::cout << "Your hand: " << hand.HandToString() << "\n";
    
    if (legalActions.empty()) { std::cout << "No legal actions. Forced PASS.\n"; return {}; }

    // Split "Pass" call
    bool isPassAllowed = false;
    std::vector<size_t> nonPassIndices;
    nonPassIndices.reserve(legalActions.size());

    for (size_t i = 0; i < legalActions.size(); ++i) {
        if (legalActions[i].empty()) isPassAllowed = true;
        else nonPassIndices.push_back(i);
    }

    bool PassAllowed = canPass && isPassAllowed;

    if (nonPassIndices.empty() && PassAllowed) {
        std::cout << "Only PASS is available.\n";
        return {};
    }

    // This part is just in case, should not happen
    if (nonPassIndices.empty() && !PassAllowed) {
        std::cout << "No legal non-PASS actions available. Forced PASS.\n";
        return {};
    }

    std::map<HandType, std::vector<size_t>> actionMap;

    for (size_t i = 0; i < legalActions.size(); ++i) {
        const auto& action = legalActions[i];
        Combination combo(action);
        HandType handtype = combo.getType();

        // In case
        if (handtype == HandType::Invalid) continue;

        actionMap[handtype].push_back(i);
    }

    struct TypeEntry {
        bool isPass;
        HandType type;
        std::string name;
        size_t count;
    };
    std::vector<TypeEntry> typeEntries;
    typeEntries.reserve(actionMap.size() + 1);

    if (PassAllowed) {
        typeEntries.push_back({true, HandType::Invalid, "PASS", 0});
    }

    for (const auto& handtype_check : actionMap){
        HandType handtype = handtype_check.first;
        const auto& indices = handtype_check.second;
        if (indices.empty()) continue;

        TypeEntry entry;
        entry.isPass = false;
        entry.type = handtype;
        entry.name = handTypetoString(handtype);
        entry.count = indices.size();
        typeEntries.push_back(entry);
    }

    // In case
    if (typeEntries.empty()) {
        size_t actionIndex = nonPassIndices.front();
        std::cout << "Grouping error. Selecting action: " << comboToString(legalActions[actionIndex]) << "\n";
        return legalActions[actionIndex];
    }

    // Step 1: Choose type
    std::cout << "Available action types:\n";
    for (size_t i = 0; i < typeEntries.size(); ++i) {
        const auto& entry = typeEntries[i];
        if (entry.isPass) {
            std::cout << "\t[" << i << "] " << entry.name << "\n";
        } else {
            std::cout << "\t[" << i << "] " << entry.name << " (" << entry.count << " options)\n";
        }
    }

    std::cout << "Select action type index: ";
    size_t typeChoice = 0;
    if (!(std::cin >> typeChoice)){
        std::cout << "Invalid input. Defaulting to PASS if allowed; otherwise, first action.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (PassAllowed) return {};

        size_t actionIndex = nonPassIndices.front();
        return legalActions[actionIndex];
    }

    if (typeChoice >= typeEntries.size()) {
        std::cout << "Invalid choice. Defaulting to PASS if allowed; otherwise, first action.\n";
        if (PassAllowed) return {};

        size_t actionIndex = nonPassIndices.front();
        return legalActions[actionIndex];
    }

    const auto& selectedType = typeEntries[typeChoice];
    if (selectedType.isPass) return {};

    // Step 2: Choose specific action
    const auto& possibleIndices = actionMap[selectedType.type];
    if (possibleIndices.empty()) {
        std::cout << "No actions available for the selected type. Defaulting to PASS if allowed; otherwise, first action.\n";
        if (PassAllowed) return {};

        size_t actionIndex = nonPassIndices.front();
        return legalActions[actionIndex];
    }

    std::cout << "Available actions for " << selectedType.name << ":\n";
    for (size_t i = 0; i < possibleIndices.size(); ++i) {
        size_t actionIndex = possibleIndices[i];
        std::cout << "\t[" << i << "] " << comboToString(legalActions[actionIndex]) << "\n";
    }

    std::cout << "Select action index: ";
    size_t actionChoice = 0;
    if (!(std::cin >> actionChoice)){
        std::cout << "Invalid input. Defaulting to first action.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        size_t actionIndex = possibleIndices.front();
        return legalActions[actionIndex];
    }

    if (actionChoice >= possibleIndices.size()) {
        std::cout << "Invalid choice. Defaulting to first action.\n";
        size_t actionIndex = possibleIndices.front();
        return legalActions[actionIndex];
    }

    size_t finalActionIndex = possibleIndices[actionChoice];
    return legalActions[finalActionIndex];
}

std::string HumanPlayer::getTypeName() const { return "Human"; }

/* AIPlayer Implementation */

AIPlayer::AIPlayer(int seatIndex, int DB_id) : Player(seatIndex, DB_id) {}

std::vector<Card> AIPlayer::playTurn(const Combination& lastPlay, const std::vector<std::vector<Card>>& legalActions, bool canPass) {
    (void)lastPlay; // Unused parameter for now

    if (legalActions.empty()) return {};

    // Randomly select a legal action
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, legalActions.size() - 1);
    size_t choice = dist(rng);

    return legalActions[choice];
}

std::string AIPlayer::getTypeName() const { return "AI"; }