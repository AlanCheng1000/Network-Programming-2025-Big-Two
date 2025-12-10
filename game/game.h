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
    std::vector<bool> passedRound;
    bool gameOver;
    int winnerIndex;
    std::vector<bool> activePlayers;
    std::vector<PlayerType> playerTypes;
    std::vector<int> seatDB_IDs;
    std::vector<bool> isOnline;
    std::vector<Card> getHumanPlayerTimeOut(
        int playerIndex,
        const std::vector<std::vector<Card>>& legalActions,
        bool canPass,
        bool& timedOut,
        int timeLimitSeconds = 30
    );

    bool isLegalFollow(const std::vector<Card>& move) const;
    bool ClearRound() const;
    int findStartingPlayer() const;
    int nextActivePlayer(int from) const;
    int activeCount() const;
    void resetRound();
    void checkValidPlay(const std::vector<Card>& move);
    bool removeFromHand(int playerIndex, const std::vector<Card>& move);
public:
    Game();

    void initializePlayers(const std::vector<PlayerType>& types);
    void startGame();
    bool takeTurn();
    void runAutoRound(int maxTurns = 1000);
    void nextTurn();
    bool isGameOver() const;
    void setSeatOnline(int seatIndex, bool is_online);
    bool isSeatOnline(int seatIndex) const;
    int getWinner() const;
    void displayGameState() const;
    void setActivePlayers(const std::vector<bool>& active);
    int getCurrentPlayer() const;
    const Hand& getPlayerHand(int playerIndex) const;
    const Combination& getLastPlay() const;
    void setLastPlay(const Combination& combo);
    void setSeatDB_IDs(const std::vector<int>& db_ids);
    std::vector<std::unique_ptr<Player>>& getPlayers();
    std::vector<std::vector<Card>> getLegalActions(int playerIndex) const;
    GameResult getGameResult() const;
};

#endif // GAME_H