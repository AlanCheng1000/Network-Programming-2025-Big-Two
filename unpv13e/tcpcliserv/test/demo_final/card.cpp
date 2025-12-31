#include "card.h"
#include <map>
#include <stdexcept>

Card::Card(Suit suit, Rank rank) {
    this->suit = suit;
    this->rank = rank;
}

Suit Card::getsuit() const {
    return suit;
}

Rank Card::getrank() const {
    return rank;
}

std::string Card::CardToString() const {
    static const std::string suits[] = {"C", "D", "H", "S"};
    static const std::string ranks[] = {"3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2"};
    // Convert card to string form "<rank><suit>"
    return ranks[rank] + suits[suit];
}

Card Card::StringToCard(const std::string& str) {
    // Maps config
    static const std::map<std::string, Suit> suitMap = {{"C", Clubs}, {"D", Diamonds}, {"H", Hearts}, {"S", Spades}};
    static const std::map<std::string, Rank> rankMap = {{"3", Three}, {"4", Four}, {"5", Five}, {"6", Six}, {"7", Seven},
                                                        {"8", Eight}, {"9", Nine}, {"10", Ten}, {"J", Jack}, {"Q", Queen},
                                                        {"K", King}, {"A", Ace}, {"2", Two}};
    // Revert string form "<rank><suit>" back to Card object
    if (str.length() < 2 || str.length() > 3) throw std::invalid_argument("Invalid card string: " + str + " (Length error)");
    std::string rankStr = str.substr(0, str.length() - 1);
    std::string suitStr = str.substr(str.length() - 1, 1);
    try {
        return Card(suitMap.at(suitStr), rankMap.at(rankStr));
    } catch (std::out_of_range&) {
        throw std::invalid_argument("Invalid card string: " + str + " (Mapping error)");
    }
}

bool Card::operator<(const Card& other) const {
    if (rank != other.rank) return rank < other.rank;
    return suit < other.suit;
}

bool Card::operator==(const Card& other) const {
    return rank == other.rank && suit == other.suit;
}