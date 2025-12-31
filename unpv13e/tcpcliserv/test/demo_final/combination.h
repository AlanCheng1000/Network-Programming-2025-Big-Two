#ifndef COMBINATION_H
#define COMBINATION_H

#include <vector>
#include "card.h"
#include "hand.h"

enum class HandType {Invalid, Single, Pair, Straight, FullHouse, FourOfAKind, StraightFlush};
enum class Mode {Strict, Flexible};

class Combination final {
private:
    std::vector<Card> cards;
    HandType type;
    Mode mode;

    bool isValidCombination();
    HandType evaluateHandType();
    bool isStraight() const;
    bool isFlush() const;
    bool isFullHouse() const;
    bool isFourOfAKind() const;
public:
    Combination();
    explicit Combination(const std::vector<Card>& cards, Mode mode = Mode::Strict);
    HandType getType() const { return type; }
    const std::vector<Card>& getCards() const { return cards; }
    static HandType eval(const std::vector<Card>& cards);
    static bool compare(const Combination& a, const Combination& b);
};

#endif // COMBINATION_H