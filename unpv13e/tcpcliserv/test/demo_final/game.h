#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <memory>
#include "deck.h"
#include "hand.h"
#include "player.h"
#include "combination.h"

struct PlayerResult{
    int DB_Player_id;
    int seatIndex;
    bool isWinner;
};

struct GameResult{
    std::vector<PlayerResult> results;
    int winner_DB_id;
};

class Game {
private:
    static const int TOTAL_PLAYERS = 4;
    Deck deck;
    std::vector<std::unique_ptr<Player>> players;
    Combination lastPlay;
    int lastPlayer;
    int currentPlayer;
    bool firstPlay;
    int currentRound;
    bool gameOver;
    int winnerIndex;
    std::vector<bool> activePlayers;

    bool isLegalFollow(const std::vector<Card>& move) const;
    int nextActivePlayer(int from) const;
    int activeCount() const;
    bool ClearRound() const;
public:
    Game();
    void resetRound();
    std::vector<bool> passedRound;
    int findStartingPlayer() const;
    void checkValidPlay(const std::vector<Card>& move);
    bool removeFromHand(int playerIndex, const std::vector<Card>& move);

    void startGame();
    void startHumanGame();
    bool takeTurn();
    void runAutoRound(int maxTurns = 1000);
    void nextTurn();
    bool isGameOver() const;
    int getWinner() const;
    void displayGameState() const;
    void displayGameStateForPlayer(int player_seat, const char *player_name, char *buffer) const;
    void setActivePlayers(const std::vector<bool>& active);
    int getCurrentPlayer() const;
    const Hand& getPlayerHand(int playerIndex) const;
    const Combination& getLastPlay() const;
    void setLastPlay(const Combination& combo);
    std::vector<std::unique_ptr<Player>>& getPlayers();
    std::vector<std::vector<Card>> getLegalActions(int playerIndex) const;
    GameResult getGameResult() const;
};

#endif // GAME_H
