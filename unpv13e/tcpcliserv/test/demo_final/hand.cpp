#include "hand.h"
#include <algorithm>

Hand::Hand() {
    // Big 2: max 13 cards in hand.
    cards.reserve(13);
}

void Hand::addCard(const Card& card) {
    cards.push_back(card);
}

bool Hand::removeCard(const Card& card) {
    auto it = std::find(cards.begin(), cards.end(), card);
    if (it != cards.end()) {
        cards.erase(it);
        return true;
    }
    return false;
}

void Hand::sortHand() {
    std::sort(cards.begin(), cards.end());
}

size_t Hand::HandSize() const {
    return cards.size();
}

const std::vector<Card>& Hand::getCards() const {
    return cards;
}

std::string Hand::HandToString() const {
    std::string result;
    for (const auto& card : cards) {
        if (!result.empty()) result += " ";
        result += card.CardToString();
    }
    return result;
}