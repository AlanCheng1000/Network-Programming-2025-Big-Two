#ifndef CARD_H
#define CARD_H

#include <iostream>
#include <string>

enum Suit {Clubs, Diamonds, Hearts, Spades};
enum Rank {Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace, Two};

class Card final {
protected:
    Suit suit;
    Rank rank;

public:
    Card(Suit suit, Rank rank);

    Suit getsuit() const;
    Rank getrank() const;

    std::string CardToString() const;
    static Card StringToCard(const std::string& str);
    bool operator<(const Card& other) const;
    bool operator==(const Card& other) const;
};

#endif // CARD_H