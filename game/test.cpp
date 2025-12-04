#include "card.h"
#include "deck.h"
#include "combination.h"
#include "hand.h"
#include "game.h"
#include "player.h"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <random>

// Please comment the following code to run specific tests.
// int main() { std::cout << "Hello World!\n"; return 0; }

// 1. Test program for Card class, cpp file: card
// 1-1

// Test program for Card::StringToCard function.

// int main(){
//     Card c1(Suit::Clubs, Rank::Three);
//     Card c2(Suit::Spades, Rank::Ace);

//     std::cout << "Card 1: " << c1.CardToString() << std::endl;
//     std::cout << "Card 2: " << c2.CardToString() << std::endl;
//     std::cout << (c1 < c2 ? "c1 < c2" : "c1 >= c2") << std::endl;

//     return 0;
// }


// 1-2

// Test program for Card::StringToCard function.

// int main() {
//     std::vector<std::string> testInputs = {
//         "3C", "10H", "AD", "2S", "7D", "QC"
//     };

//     std::cout << "===== Testing Card::StringToCard() =====\n";

//     for (const auto& input : testInputs) {
//         try {
//             Card c = Card::StringToCard(input);
//             std::cout << "Input: " << input
//                       << "  ->  Parsed: " << c.CardToString() << "\n";
//         } catch (const std::exception& e) {
//             std::cout << "Input: " << input
//                       << "  ->  Error: " << e.what() << "\n";
//         }
//     }

//     std::cout << "===== Invalid input tests =====\n";
//     std::vector<std::string> invalidInputs = {
//         "1C", "15H", "XD", "QQ", "AA", "H3", "S13", "??"
//     };

//     for (const auto& input : invalidInputs) {
//         try {
//             Card c = Card::StringToCard(input);
//             std::cout << "Should have failed for: " << input
//                       << "  but got: " << c.CardToString() << "\n";
//         } catch (const std::exception& e) {
//             std::cout << "Caught expected error for \"" << input
//                       << "\": " << e.what() << "\n";
//         }
//     }

//     return 0;
// }


// 2. Test program for Deck class, cpp file: deck, card

// int main() {
//     std::cout << "===== Testing Deck =====\n";

//     Deck deck;
//     deck.reset();
//     std::cout << "Deck initialized.\n";

//     std::cout << "Test1: remainingCards()\n";
//     if (deck.remainingCards() != 52) {
//         std::cout << "Error: Expected 52 cards, got " << deck.remainingCards() << "\n";
//         return 1;
//     } else {
//         std::cout << "Passed: Deck has 52 cards.\n";
//     }

//     std::cout << "Test2: shuffle()\n";
//     deck.reset();
//     std::vector<Card> beforeShuffle;
//     for (size_t i = 0; i < 52; ++i) beforeShuffle.push_back(deck.dealCard());

//     deck.reset();
//     deck.shuffle();
//     std::vector<Card> afterShuffle;
//     for (size_t i = 0; i < 52; ++i) afterShuffle.push_back(deck.dealCard());

//     int sameCount = 0;
//     for (size_t i = 0; i < 52; ++i) {
//         if (beforeShuffle[i].CardToString() == afterShuffle[i].CardToString())
//             sameCount++;
//     }

//     std::cout << "Same position count after shuffle: " << sameCount << " / 52\n";
//     if (sameCount < 10)
//         std::cout << "Passed: Shuffle randomized deck order.\n";
//     else
//         std::cout << "Warning: Deck may not be shuffled well.\n";

//     std::cout << "Test3: dealCard()\n";
//     deck.reset();
//     deck.shuffle();

//     std::set<std::string> seen;
//     bool duplicate = false;
//     for (int i = 0; i < 52; ++i) {
//         Card c = deck.dealCard();
//         std::string s = c.CardToString();
//         if (!seen.insert(s).second) {
//             std::cout << "Duplicate found: " << s << "\n";
//             duplicate = true;
//             break;
//         }
//     }

//     if (!duplicate && deck.remainingCards() == 0) {
//         std::cout << "Passed: All 52 unique cards dealt correctly.\n";
//     } else {
//         std::cout << "Error: Deck dealing inconsistency.\n";
//     }

//     std::cout << "Test4: dealCard() beyond 52 cards\n";
//     try {
//         deck.dealCard(); // 53rd card
//         std::cout << "Error: No exception thrown after 52 cards.\n";
//     } catch (const std::out_of_range&) {
//         std::cout << "Passed: Exception correctly thrown on extra deal.\n";
//     }

//     std::cout << "===== Deck test complete =====\n";
//     return 0;
// }


// 3. Test program for Combination class, cpp file: combination, card

// std::string handTypeToString(HandType type) {
//     switch (type) {
//         case HandType::Invalid: return "Invalid";
//         case HandType::Single: return "Single";
//         case HandType::Pair: return "Pair";
//         case HandType::Straight: return "Straight";
//         case HandType::Flush: return "Flush";
//         case HandType::FullHouse: return "FullHouse";
//         case HandType::FourOfAKind: return "FourOfAKind";
//         case HandType::StraightFlush: return "StraightFlush";
//         default: return "Unknown";
//     }
// }

// void printCombination(const std::vector<Card>& cardStrs) {
//     for (const auto& card : cardStrs) std::cout << card.CardToString() << " ";
//     std::cout << "\n";
// }

// int main() {
//     std::cout << "===== Combination Class Test =====\n";

//     // Test 1: Single
//     std::cout << "Test 1: Single\n";
//     std::vector<Card> single = { Card(Suit::Hearts, Rank::Three) };
//     Combination c1(single);
//     std::cout << "[Single] "; printCombination(single);
//     std::cout << "Result: " << handTypeToString(c1.getType()) << "\n\n";

//     // Test 2: Pair
//     std::cout << "Test 2: Pair\n";
//     std::vector<Card> pair = { Card(Suit::Hearts, Rank::Three), Card(Suit::Diamonds, Rank::Three) };
//     Combination c2(pair);
//     std::cout << "[Pair] "; printCombination(pair);
//     std::cout << "Result: " << handTypeToString(c2.getType()) << "\n\n";

//     // Test 3: Straight
//     std::cout << "Test 3: Straight\n";
//     std::vector<Card> straight = {
//         Card(Suit::Hearts, Rank::Three), Card(Suit::Diamonds, Rank::Four),
//         Card(Suit::Clubs, Rank::Five), Card(Suit::Spades, Rank::Six),
//         Card(Suit::Hearts, Rank::Seven)
//     };
//     Combination c3(straight);
//     std::cout << "[Straight] "; printCombination(straight);
//     std::cout << "Result: " << handTypeToString(c3.getType()) << "\n\n";

//     // Test 4: Flush
//     std::cout << "Test 4: Flush\n";
//     std::vector<Card> flush = {
//         Card(Suit::Hearts, Rank::Two), Card(Suit::Hearts, Rank::Three),
//         Card(Suit::Hearts, Rank::Four), Card(Suit::Hearts, Rank::Nine),
//         Card(Suit::Hearts, Rank::Seven)
//     };
//     Combination c4(flush);
//     std::cout << "[Flush] "; printCombination(flush);
//     std::cout << "Result: " << handTypeToString(c4.getType()) << "\n\n";

//     // Test 5: Full House
//     std::cout << "Test 5: Full House\n";
//     std::vector<Card> fullHouse = {
//         Card(Suit::Hearts, Rank::Three), Card(Suit::Spades, Rank::Three),
//         Card(Suit::Diamonds, Rank::Three), Card(Suit::Clubs, Rank::Four),
//         Card(Suit::Spades, Rank::Four)
//     };
//     Combination c5(fullHouse);
//     std::cout << "[Full House] "; printCombination(fullHouse);
//     std::cout << "Result: " << handTypeToString(c5.getType()) << "\n\n";

//     // Test 6: Four of a Kind
//     std::cout << "Test 6: Four of a Kind\n";
//     std::vector<Card> fourOfAKind = {
//         Card(Suit::Hearts, Rank::Three), Card(Suit::Spades, Rank::Three),
//         Card(Suit::Diamonds, Rank::Three), Card(Suit::Clubs, Rank::Three),
//         Card(Suit::Hearts, Rank::Four)
//     };
//     Combination c6(fourOfAKind);
//     std::cout << "[Four of a Kind] "; printCombination(fourOfAKind);
//     std::cout << "Result: " << handTypeToString(c6.getType()) << "\n\n";

//     // Test 7: Straight Flush
//     std::cout << "Test 7: Straight Flush\n";
//     std::vector<Card> straightFlush = {
//         Card(Suit::Hearts, Rank::Three), Card(Suit::Hearts, Rank::Four),
//         Card(Suit::Hearts, Rank::Five), Card(Suit::Hearts, Rank::Six),
//         Card(Suit::Hearts, Rank::Seven)
//     };
//     Combination c7(straightFlush);
//     std::cout << "[Straight Flush] "; printCombination(straightFlush);
//     std::cout << "Result: " << handTypeToString(c7.getType()) << "\n\n";

//     // Test 8: Compare()
//     std::cout << "Test 8: Compare Combinations\n";
//     std::cout << "Result: " << (Combination::compare(c7, c5) ? "StraightFlush Wins" : "FullHouse Wins or Invalid") << "\n";

//     std::cout << "===== Combination Test Complete =====\n";
//     return 0;
// }


// 4. Test program for Hand class, cpp file: hand, deck, card

// int main() {
//     std::cout << "===== Hand Class Test =====\n";

//     Hand hand;

//     // Test 1: Add cards
//     std::cout << "Test 1: Add cards\n";
//     hand.addCard(Card(Suit::Hearts, Rank::Three));
//     hand.addCard(Card(Suit::Clubs, Rank::Ace));
//     hand.addCard(Card(Suit::Spades, Rank::Ten));
//     hand.addCard(Card(Suit::Diamonds, Rank::Five));
//     hand.addCard(Card(Suit::Hearts, Rank::King));
//     std::cout << "Current hand: " << hand.HandToString() << "\n";
//     std::cout << "Hand size: " << hand.HandSize() << "\n";

//     // Test 2: Remove a card
//     std::cout << "Test 2: Remove a card\n";
//     bool removed = hand.removeCard(Card(Suit::Spades, Rank::Ten));
//     std::cout << (removed ? "Removed 10S from hand.\n" : "Failed to remove 10S from hand.\n");
//     std::cout << "Hand after removal: " << hand.HandToString() << "\n";
//     std::cout << "Hand size: " << hand.HandSize() << "\n";

//     // Test 3: Sort hand
//     std::cout << "Test 3: Sort hand\n";
//     std::cout << "Hand before sorting: " << hand.HandToString() << "\n";
//     hand.sortHand();
//     std::cout << "Hand after sorting: " << hand.HandToString() << "\n";

//     // Test 4: Rehearse a complete hand
//     std::cout << "Test 4: Rehearse complete hand operations\n";
//     Deck deck;
//     deck.shuffle();
//     while (hand.HandSize() < 13) {
//         hand.addCard(deck.dealCard());
//     }
//     std::cout << "Full hand: " << hand.HandToString() << "\n";
//     std::cout << "Hand size: " << hand.HandSize() << "\n";

//     // Test 5: getCards()
//     std::cout << "Test 5: getCards()\n";
//     const std::vector<Card>& handRef = hand.getCards();
//     std::cout << "The first three cards in hand via getCards(): ";
//     for (int i = 0; i < 3 && i < handRef.size(); ++i) {
//         std::cout << handRef[i].CardToString() << " ";
//     }
//     std::cout << "\n";

//     std::cout << "===== Hand Test Complete =====\n";
//     return 0;
// }


// 5. Test program for Player and Game class, cpp file: player, hand, card, combination, game, deck

int main() {
    std::cout << "===== Player Integration Test =====\n";

    Game game;
    game.startGame();

    int playcard = 1;
    while (!game.isGameOver()) {
        std::cout << "\n--- Play count: " << playcard++ << " ---\n";
        game.displayGameState();
        game.takeTurn();
    }

    std::cout << "\n===== Test Finished =====\n";
    return 0;
}

// compile command:
// ================================= Generate Human Player Test exe ====================================
// Check if game.cpp has been modified to use HumanPlayer in startGame() function.
// g++ test.cpp card.cpp deck.cpp combination.cpp hand.cpp player.cpp game.cpp -o test_Human -std=c++17
// =====================================================================================================
//
// ================================== Generate AI Player Test exe ==================================
// Check if game.cpp has been modified to use AIPlayer in startGame() function.
// g++ test.cpp card.cpp deck.cpp combination.cpp hand.cpp player.cpp game.cpp -o test_AI -std=c++17
// =================================================================================================