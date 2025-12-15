#ifndef DECK_H
#define DECK_H

#include <vector>
#include "card.h"

class Deck final {
private:
    std::vector<Card> cards;
    size_t currentIndex;
public:
    Deck();

    void reset();
    void shuffle();
    Card dealCard();
    size_t remainingCards() const;
};

#endif // DECK_H