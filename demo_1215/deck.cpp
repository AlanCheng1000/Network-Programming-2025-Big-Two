#include "deck.h"
#include <random>
#include <algorithm>
#include <stdexcept>

Deck::Deck() {
    reset();
}

void Deck::reset() {
    cards.clear();
    for (int s = Clubs; s <= Spades; ++s) {
        for (int r = Three; r <= Two; ++r) {
            cards.emplace_back(static_cast<Suit>(s), static_cast<Rank>(r));
        }
    }
    currentIndex = 0;
}

void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
    currentIndex = 0;
}

Card Deck::dealCard() {
    if (currentIndex >= cards.size()) {
        throw std::out_of_range("No more cards in the deck.");
    }
    return cards[currentIndex++];
}

size_t Deck::remainingCards() const {
    return cards.size() - currentIndex;
}