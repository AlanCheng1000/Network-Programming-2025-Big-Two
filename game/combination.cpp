#include "combination.h"
#include <algorithm>
#include <map>
#include <array>
#include <stdexcept>

namespace {
    inline int suitWeight(Suit suit) { return static_cast<int>(suit); }

    /*
        The manipulated order of straight ranks:
        3, 4, 5, 6, 2
        10, J, Q, K, A
        9, 10, J, Q, K
        8, 9, 10, J, Q
        7, 8, 9, 10, J
        6, 7, 8, 9, 10
        5, 6, 7, 8, 9
        4, 5, 6, 7, 8
        3, 4, 5, 6, 7
        A, 2, 3, 4, 5

        The last element in each array is considered the highest rank in that straight.
    */

    static const std::array<std::array<Rank, 5>, 10> Straight_Order = {{
        { Rank::Three,      Rank::Four,     Rank::Five,     Rank::Six,      Rank::Two },
        { Rank::Ten,        Rank::Jack,     Rank::Queen,    Rank::King,     Rank::Ace },
        { Rank::Nine,       Rank::Ten,      Rank::Jack,     Rank::Queen,    Rank::King },
        { Rank::Eight,      Rank::Nine,     Rank::Ten,      Rank::Jack,     Rank::Queen },
        { Rank::Seven,      Rank::Eight,    Rank::Nine,     Rank::Ten,      Rank::Jack },
        { Rank::Six,        Rank::Seven,    Rank::Eight,    Rank::Nine,     Rank::Ten },
        { Rank::Five,       Rank::Six,      Rank::Seven,    Rank::Eight,    Rank::Nine },
        { Rank::Four,       Rank::Five,     Rank::Six,      Rank::Seven,    Rank::Eight },
        { Rank::Three,      Rank::Four,     Rank::Five,     Rank::Six,      Rank::Seven },
        { Rank::Ace,        Rank::Two,      Rank::Three,    Rank::Four,     Rank::Five }
    }};

    inline bool straightPatternWeight(const std::vector<Card>& cards, int& outWeight, Suit& TopSuit) {
        if (cards.size() != 5) return false;

        // Check if all five ranks are in any of the straight patterns
        std::map<Rank, int> rankCount;
        for (const auto& card : cards) rankCount[card.getrank()]++;

        for (int idx = 0; idx < static_cast<int>(Straight_Order.size()); ++idx) {
            const auto& pattern = Straight_Order[idx];
            bool match = true;
            for (Rank rank: pattern) {
                auto it = rankCount.find(rank);
                if (it == rankCount.end() || it->second != 1) { match = false; break; }
            }
            if (!match) continue;

            // Calculate weight and top suit
            Rank topRank = pattern[4];
            Suit topSuit = Suit::Clubs;
            for (const auto& card : cards) {
                if (card.getrank() == topRank) { topSuit = card.getsuit(); break;}
            }

            TopSuit = topSuit;
            outWeight = 10 - idx; // Higher index means lower weight
            return true;
        }
        
        return false;
    }

    inline std::map<Rank, int> RankCounts(const std::vector<Card>& cards) {
        std::map<Rank, int> rankCount;
        for (const auto& card : cards) rankCount[card.getrank()]++;
        return rankCount;
    }

    inline bool isFH(const std::map<Rank, int>& rankCount) {
        if (rankCount.size() != 2) return false;
        bool hasThree = false, hasTwo = false;
        for (const auto& entry : rankCount) {
            if (entry.second == 3) hasThree = true;
            else if (entry.second == 2) hasTwo = true;
            else return false;
        }
        return hasThree && hasTwo;
    }

    inline Rank FullHouseTripleRank(const std::map<Rank, int>& cards) {
        if (!isFH(cards)) throw std::runtime_error("Not a Full House");
        for (const auto& entry : cards) if (entry.second == 3) return entry.first;
        throw std::runtime_error("Not a Full House");
    }

    inline bool isFOAK(const std::map<Rank, int>& rankCount) {
        if (rankCount.size() != 2) return false;
        for (const auto& entry : rankCount) {
            if (entry.second == 4) return true;
        }
        return false;
    }

    inline Rank FourOfAKindRank(const std::map<Rank, int>& cards) {
        if (!isFOAK(cards)) throw std::runtime_error("Not a Four Of A Kind");
        for (const auto& entry : cards) if (entry.second == 4) return entry.first;
        throw std::runtime_error("Not a Four Of A Kind");
    }
}

Combination::Combination() : cards(), type(HandType::Invalid), mode(Mode::Strict) {}

Combination::Combination(const std::vector<Card>& cards, Mode mode) : cards(cards), mode(mode) {
    if (!isValidCombination()) {
        type = HandType::Invalid;
    } else {
        type = evaluateHandType();
    }
}

HandType Combination::eval(const std::vector<Card>& cards) {
    Combination temp(cards);
    return temp.getType();
}

bool Combination::compare(const Combination& a, const Combination& b) {
    if(a.type == HandType::Invalid || b.type == HandType::Invalid) {
        return false; // Invalid combinations cannot be compared
    }

    // Compare cards by rank and suit if they are of the same type
    if (a.type == b.type) {
        // For type with 5 cards
        if (a.cards.size() == 5){
            switch (a.type) {
                case HandType::Straight:
                case HandType::StraightFlush: {
                    int a_weight = 0, b_weight = 0;
                    Suit a_topSuit = Suit::Clubs, b_topSuit = Suit::Clubs;
                    bool a_isStraight = straightPatternWeight(a.cards, a_weight, a_topSuit);
                    bool b_isStraight = straightPatternWeight(b.cards, b_weight, b_topSuit);
            
                    if (!a_isStraight || !b_isStraight) return false; // Should not happen for valid straights
                    if (a_weight != b_weight) return a_weight > b_weight;
                    return suitWeight(a_topSuit) > suitWeight(b_topSuit);
                }

                case HandType::FullHouse: {
                    auto a_rankCount = RankCounts(a.getCards());
                    auto b_rankCount = RankCounts(b.getCards());
                    Rank a_tripleRank = FullHouseTripleRank(a_rankCount);
                    Rank b_tripleRank = FullHouseTripleRank(b_rankCount);
                    if (a_tripleRank != b_tripleRank) return a_tripleRank > b_tripleRank;
                }

                case HandType::FourOfAKind: {
                    auto a_rankCount = RankCounts(a.getCards());
                    auto b_rankCount = RankCounts(b.getCards());
                    Rank a_quadRank = FourOfAKindRank(a_rankCount);
                    Rank b_quadRank = FourOfAKindRank(b_rankCount);
                    if (a_quadRank != b_quadRank) return a_quadRank > b_quadRank;
                }

                default: break;
            }
        }

        // Other types: compare highest card
        std::vector<Card> ca = a.getCards();
        std::vector<Card> cb = b.getCards();
        std::sort(ca.begin(), ca.end());
        std::sort(cb.begin(), cb.end());
        return cb.back() < ca.back();
    }

    // Different types
    if (a.mode == Mode::Strict) {
        if (a.type == HandType::StraightFlush || a.type == HandType::FourOfAKind) return true;
        return false;
    } else{
        return static_cast<int>(a.type) > static_cast<int>(b.type);
    }
}

bool Combination::isValidCombination() {
    size_t n = cards.size();

    // Valid sizes: 1 (Single), 2 (Pair), 5 (Various 5-card hands)
    if (n != 1 && n != 2 && n != 5) return false;

    // Exclusions for duplicated cards
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (cards[i] == cards[j]) return false;
        }
    }

    // Check pairs have same rank
    if (n == 2 && cards[0].getrank() != cards[1].getrank()) return false;

    return true;
}

HandType Combination::evaluateHandType() {
    size_t n = cards.size();
    if (n == 1) return HandType::Single;
    if (n == 2) return HandType::Pair;

    if (n == 5) {
        if (isStraight() && isFlush()) return HandType::StraightFlush;
        if (isFourOfAKind()) return HandType::FourOfAKind;
        if (isFullHouse()) return HandType::FullHouse;
        if (isStraight()) return HandType::Straight;
    }

    return HandType::Invalid;
}

// Check for the 5-card hands
bool Combination::isStraight() const {
    // Valid straights check
    int weight = 0;
    Suit topSuit = Suit::Clubs;
    return straightPatternWeight(cards, weight, topSuit);
}

bool Combination::isFlush() const {
    // Preliminary checks
    if (cards.size() != 5) return false;

    Suit firstSuit = cards[0].getsuit();

    // Check if all cards have the same suit
    for (const auto& card : cards) {
        if (card.getsuit() != firstSuit) return false;
    }
    return true;
}

bool Combination::isFullHouse() const {
    // Preliminary checks
    if (cards.size() != 5) return false;
    auto rankCount = RankCounts(cards);
    return isFH(rankCount);
}

bool Combination::isFourOfAKind() const {
    // Preliminary checks
    if (cards.size() != 5) return false;
    auto rankCount = RankCounts(cards);
    return isFOAK(rankCount);
}