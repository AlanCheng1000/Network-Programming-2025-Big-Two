#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include "hand.h"
#include "combination.h"

enum class PlayerType {Human, AI};

class Player {
protected:
    int seatIndex; // Player seat index (0-3)
    int DB_id;    // Database ID
    Hand hand;
    bool isActive;
public:
    explicit Player(int seatIndex, int DB_id = -1);
    virtual ~Player() = default;

    int getseatIndex() const;   // Get player seat index
    int getDBId() const;        // Get database ID
    void setDBId(int id);

    Hand& getHand();
    const Hand& getHand() const;
    bool isPlayerActive() const;

    void deactivate();
    void addCard(const Card& card);

    virtual std::vector<Card> playTurn(const Combination& lastPlay, const std::vector<std::vector<Card>>& legalActions, bool canPass) = 0;
    virtual std::string getTypeName() const = 0;
};

class HumanPlayer final : public Player {
public:
    explicit HumanPlayer(int seatIndex, int DB_id = -1);
    std::vector<Card> playTurn(const Combination& lastPlay, const std::vector<std::vector<Card>>& legalActions, bool canPass) override;
    std::string getTypeName() const override;
};

class AIPlayer final : public Player {
public:
    explicit AIPlayer(int seatIndex, int DB_id = -1);
    std::vector<Card> playTurn(const Combination& lastPlay, const std::vector<std::vector<Card>>& legalActions, bool canPass) override;
    std::string getTypeName() const override;
};

# endif // PLAYER_H