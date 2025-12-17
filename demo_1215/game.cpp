#include "game.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <cstring>
#include <sstream>

extern "C"{
  #include "../../../lib/unp.h"
}

namespace {
    inline bool has3C(const std::vector<Card>& v){
        for (const auto& c : v)
            if (c.getrank()==Rank::Three && c.getsuit()==Suit::Clubs) return true;
        return false;
    }

    inline std::string comboToString(const std::vector<Card>& cards){
        std::string s;
        for (size_t i = 0; i < cards.size(); ++i){
            if (i) s += ' ';
            s += cards[i].CardToString();
        }
        return s.empty() ? std::string("-") : s;
    }

}

Game::Game() : lastPlay(), lastPlayer(-1), currentPlayer(0), firstPlay(true), currentRound(1), passedRound(TOTAL_PLAYERS, false), gameOver(false), winnerIndex(-1)
    , activePlayers(TOTAL_PLAYERS, true) {}


void Game::startGame() {
    players.clear();
    for (int i = 0; i < TOTAL_PLAYERS; ++i)
        players.emplace_back(std::make_unique<AIPlayer>(i));

    deck.reset();
    deck.shuffle();

    for (int i = 0; i < 13; ++i) {
        for (auto& p : players) p->addCard(deck.dealCard());
    }
    for (auto& p : players) p->getHand().sortHand();

    lastPlay     = Combination();
    lastPlayer   = -1;
    firstPlay    = true;
    currentRound = 1;
    gameOver     = false;
    winnerIndex  = -1;
    passedRound.assign(TOTAL_PLAYERS, false);

    // Find 3C holder
    currentPlayer = findStartingPlayer();
    std::cout << "Player " << currentPlayer+1 << " holds 3C and starts the first play.\n";
}

void Game::startHumanGame() {
    players.clear();
    for (int i = 0; i < TOTAL_PLAYERS; ++i)
        players.emplace_back(std::make_unique<HumanPlayer>(i));

    deck.reset();
    deck.shuffle();

    for (int i = 0; i < 13; ++i) {
        for (auto& p : players) p->addCard(deck.dealCard());
    }
    for (auto& p : players) p->getHand().sortHand();

    lastPlay     = Combination();
    lastPlayer   = -1;
    firstPlay    = true;
    currentRound = 1;
    gameOver     = false;
    winnerIndex  = -1;
    passedRound.assign(TOTAL_PLAYERS, false);

    // Find 3C holder
    currentPlayer = findStartingPlayer();
    std::cout << "Player " << currentPlayer+1 << " holds 3C and starts the first play.\n";
}

int Game::findStartingPlayer() const {
    for (int i = 0; i < TOTAL_PLAYERS; ++i) {
        const auto& cards = players[i]->getHand().getCards();
        for (const auto& c : cards){
            if (c.getrank() == Rank::Three && c.getsuit() == Suit::Clubs) return i;
        }
    }
    return 0;
}

int Game::activeCount() const {
    int count = 0;
    for (int i = 0; i < TOTAL_PLAYERS; ++i)
        if (activePlayers[i]) ++count;
    return count;
}

int Game::nextActivePlayer(int from) const {
    int i = from;
    for(;;){
        i = (i + 1) % TOTAL_PLAYERS;
        if (!activePlayers[i]) continue;

        const bool newRound = (lastPlay.getType() == HandType::Invalid || lastPlay.getCards().empty());
        if (!newRound && passedRound[i]) continue;

        return i;
    }
}

bool Game::removeFromHand(int playerIndex, const std::vector<Card>& move){
    auto& hand = players[playerIndex]->getHand();
    for (const auto& c : move){
        if (!hand.removeCard(c)){
            std::cout << "Card not in hand: " << c.CardToString() << "\n";
            return false;
        }
    }
    return true;
}

bool Game::ClearRound() const {
    if (lastPlayer == -1) return false;

    int notPassed = 0;
    int lastNotPassedIndex = -1;

    for (int i = 0; i < TOTAL_PLAYERS; ++i) {
        if (!activePlayers[i]) continue;
        if (!passedRound[i]) {
            ++notPassed;
            lastNotPassedIndex = i;
        }
    }

    return notPassed == 1 && lastNotPassedIndex == lastPlayer;
}

void Game::resetRound(){
    ++currentRound;
    std::cout << "Current round ends. Player " << lastPlayer + 1 << " leads a new round.\n";
    lastPlay    = Combination(); // Clear combination

    passedRound.assign(TOTAL_PLAYERS, false);
    currentPlayer = lastPlayer;  // Last player leads next round
}

void Game::checkValidPlay(const std::vector<Card>& move){
    if (!removeFromHand(currentPlayer, move)){ // Should not happen if isLegalFollow is used properly. Just in case.
        return;
    }
    lastPlay   = Combination(move);
    lastPlayer = currentPlayer;


    passedRound[currentPlayer] = false;

    if (firstPlay) firstPlay = false;

    std::cout << "Player " << currentPlayer + 1 << " played: " << comboToString(move) << "\n";

    if (players[currentPlayer]->getHand().HandSize() == 0){
        gameOver = true;
        winnerIndex = currentPlayer;
        std::cout << "\nPlayer " << winnerIndex + 1 << " wins the game!\n";
    } else {
        nextTurn();
    }
}

bool Game::isLegalFollow(const std::vector<Card>& move) const {
    const bool newRound = (lastPlay.getType() == HandType::Invalid || lastPlay.getCards().empty());

    // PASS
    if (move.empty()){
        // The first play of a new round cannot be PASS
        if (newRound) return false;
        return true;
    }

    // First check if it is a valid combination
    Combination current(move);
    if (current.getType() == HandType::Invalid) return false;

    // First play: Must contain 3C
    if (firstPlay && newRound && !has3C(move)) return false;

    // New round: Any valid combination is allowed
    if (newRound) return true;

    // Follow: Must match the number of cards
    if (move.size() != lastPlay.getCards().size()) return false;

    // If 5-card hand, must match the type
    if (move.size() == 5 && current.getType() != lastPlay.getType()) return false;

    // Finally compare the two combinations
    return Combination::compare(current, lastPlay);
}

// Game play

bool Game::takeTurn() {
    if (gameOver) return false;

    const bool newRound = (lastPlay.getType()==HandType::Invalid || lastPlay.getCards().empty());

    if (!newRound && passedRound[currentPlayer]) {
        currentPlayer = nextActivePlayer(currentPlayer);
        return true;
    }

    auto legalActions = getLegalActions(currentPlayer);

    if (legalActions.empty()) {
        std::cout << "No legal actions for Player " << currentPlayer + 1 << ". Forced PASS.\n";
        passedRound[currentPlayer] = true;

        if (ClearRound()) { resetRound(); return true; }

        nextTurn();
        return true;
    }

    bool canPass = !newRound;

    auto move = players[currentPlayer]->playTurn(lastPlay, legalActions, canPass);

    if (!isLegalFollow(move)){
        std::cout << "Illegal move by Player " << currentPlayer + 1 << ". Move treated as PASS.\n";

        passedRound[currentPlayer] = true;

        if (ClearRound()) { resetRound(); return true; }

        nextTurn();
        return false;
    }

    // PASS
    if (move.empty()){
        std::cout << "Player " << currentPlayer + 1 << " passes.\n";
        passedRound[currentPlayer] = true;

        if (ClearRound()) { resetRound(); return true; }

        nextTurn();
        return true;
    }

    checkValidPlay(move);
    return true;
}

// Get all legal actions for a player
std::vector<std::vector<Card>> Game::getLegalActions(int playerIndex) const {
    std::vector<std::vector<Card>> actions;

    if (gameOver) return actions;
    if (playerIndex < 0 || playerIndex >= TOTAL_PLAYERS) return actions;
    if (!activePlayers[playerIndex]) return actions;

    const auto& cards = players[playerIndex]->getHand().getCards();
    const int n = static_cast<int>(cards.size());

    const bool newRound = (lastPlay.getType() == HandType::Invalid || lastPlay.getCards().empty());
    const int lastPlaySize = newRound ? 0 : static_cast<int>(lastPlay.getCards().size());

    // First, consider PASS
    std::vector<Card> passMove; // Empty vector represents PASS
    if (isLegalFollow(passMove)) {
        actions.push_back(passMove);
    }

    // Then determine the total number of cards to play
    auto validSizes = [&](int size) -> bool {
        if (size != 1 && size != 2 && size != 5) return false;
        if (newRound) return true;
        return size == lastPlaySize;
    };

    // Case 1: Single cards
    if (validSizes(1)) {
        for (int i = 0; i < n; ++i) {
            std::vector<Card> move{cards[i]};
            if (isLegalFollow(move)) {
                actions.push_back(move);
            }
        }
    }

    // Case 2: Pairs
    if (validSizes(2)) {
        for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            std::vector<Card> move{cards[i], cards[j]};
            if (isLegalFollow(move)) actions.push_back(move);
        }}
    }

    // Case 3: 5-card combinations
    if (validSizes(5)) {
        // Loop starts here.
        for (int a = 0; a < n; ++a) {
        for (int b = a + 1; b < n; ++b) {
        for (int c = b + 1; c < n; ++c) {
        for (int d = c + 1; d < n; ++d) {
        for (int e = d + 1; e < n; ++e) {
            std::vector<Card> move{cards[a], cards[b], cards[c], cards[d], cards[e]};
            if (isLegalFollow(move)) actions.push_back(move);
        }}}}}   // Loop ends here.
    }

    return actions;
}

// Automated round for AI players
void Game::runAutoRound(int maxTurns){
    int turns = 0;
    while (!gameOver && turns < maxTurns){
        if (!takeTurn()) continue;
        ++turns;
    }
}

// Other complementary components

void Game::nextTurn() {
    currentPlayer = nextActivePlayer(currentPlayer);
    std::cout << "It's now player " << currentPlayer + 1 << "'s turn.\n";
}

bool Game::isGameOver() const { return gameOver; }
int  Game::getWinner()  const { return winnerIndex; }

void Game::displayGameState() const {
    std::cout << "---------- Game State ----------\n";
    for (int i = 0; i < TOTAL_PLAYERS; ++i) {
        std::cout << "Player " << i + 1 << " hand: "
                  << players[i]->getHand().HandToString();
        if (!activePlayers[i]) std::cout << " (Inactive)";
        std::cout << "\n";
    }
    std::cout << "\n";
    if (firstPlay) std::cout << "First play.\n";
    std::cout << "Current round: " << currentRound << "\n";
    std::cout << "Current player: Player " << currentPlayer + 1 << "\n";
    std::cout << "Last play: " << comboToString(lastPlay.getCards()) << "\n";
    std::cout << "Players passed this round:";
    for(int i = 0; i < TOTAL_PLAYERS; ++i){
        if (!activePlayers[i]) continue;
        if (passedRound[i]) std::cout << " " << i + 1;
    }
    std::cout << "\n";
    std::cout << "--------------------------------\n";
}

void Game::displayGameStateForPlayer(int player_seat, const char *player_name, char *buffer) const{
    std::ostringstream msg; 
    msg << "--------------------------------\n";
    msg << "| cards left |";
    for(int i = 0; i < TOTAL_PLAYERS; ++i){
        msg << " #" << i + 1 << ":" << players[i]->getHand().HandSize() << " |";
    }
    msg << "\n";
    msg << "(Abbreviations: C for Club; D for Diamond; H for Heart; S for Spades.)" << "\n";
    msg << "Player #" << player_seat + 1 << " \"" << player_name << "\" hand: "
              << players[player_seat]->getHand().HandToString();
    if (!activePlayers[player_seat]) msg << " (Inactive)";
    msg << "\n";
   
    std::cout << "\n";
    if (firstPlay) msg << "First play.\n";
    msg << "Current round: " << currentRound << "\n";
    msg << "Current player: player#" << currentPlayer + 1 << "\n";
    msg << "Last play: " << comboToString(lastPlay.getCards()) << "\n";
    msg << "Players passed this round:";
    for(int i = 0; i < TOTAL_PLAYERS; ++i){
        if (!activePlayers[i]) continue;
        if (passedRound[i]) msg << " #" << i + 1;
    }
    msg << "\n";
    msg << "--------------------------------\n";
    msg << "If it's your turn press any key to play\n";
    
    // cpp str to c-style string
    const std::string final_msg = msg.str();
    const char *result = final_msg.c_str();
    size_t size_to_copy = min(final_msg.length(), MAXLINE - 1);
    memcpy(buffer, result, size_to_copy);
    return;
}

void Game::setActivePlayers(const std::vector<bool>& active) {
    if (active.size() != TOTAL_PLAYERS) {
        throw std::invalid_argument("Active player list size must be 4.");
    }
    activePlayers = active;
    if (!activePlayers[currentPlayer]) nextTurn();
}

int Game::getCurrentPlayer() const { return currentPlayer; }

const Hand& Game::getPlayerHand(int playerIndex) const {
    if (playerIndex < 0 || playerIndex >= TOTAL_PLAYERS)
        throw std::out_of_range("Invalid player index");
    return players[playerIndex]->getHand();
}

std::vector<std::unique_ptr<Player>>& Game::getPlayers() { return players; }

const Combination& Game::getLastPlay() const { return lastPlay; }

void Game::setLastPlay(const Combination& combo) {
    lastPlay = combo;
}

GameResult Game::getGameResult() const {
    GameResult result;
    result.results.reserve(TOTAL_PLAYERS);

    int winner_DB_id = -1;
    if (winnerIndex >= 0 && winnerIndex < static_cast<int>(players.size())) winner_DB_id = players[winnerIndex]->getDBId();

    result.winner_DB_id = winner_DB_id;

    for (int seat = 0; seat < TOTAL_PLAYERS; ++seat) {
        if (!players[seat]) continue;

        PlayerResult pr;
        pr.seatIndex    = seat;
        pr.DB_Player_id = players[seat]->getDBId();
        pr.isWinner     = (seat == winnerIndex);
        result.results.push_back(pr);
    }

    return result;
}

// =====================================================
//  C-style interface for external access (Python / GUI)
// =====================================================
// omitted

