#ifndef HAND_H
#define HAND_H

#include <vector>
#include <string>
#include "card.h"

class Hand final {
private:
    std::vector<Card> cards;
public:
    Hand();

    void addCard(const Card& card);
    bool removeCard(const Card& card);
    void sortHand();
    size_t HandSize() const;
    const std::vector<Card>& getCards() const;
    std::string HandToString() const;
};

#endif // HAND_H