#define NOMINMAX
#include "database.h"
#include "game.h"
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

int main() {
    // ==========================================================================
    // |   Please run the whole code at the same time.                          |
    // |   To save time, it is recommended to compile game.cpp with AIplayer.   |
    // ==========================================================================

    std::cout << "Hello! This is the Big2 Game Module Test Program.\n\n";

    // ===============================
    // |   1. DB connection config   |
    // ===============================
    std::cout << "Part 1: DB connection config\n\n";
    if (!connect_to_DB("192.168.42.129", "NP_Big2_user", "CYYsci1121921", "big2_db", 3306)) {
        std::cerr << "Connection failed.\n";
        return 1;
    }
    std::cout << "Connection has been configured.\n\n";

    // ==================================
    // |   2. New player registration   |
    // ==================================
    std::cout << "Part 2: New player registration\n\n";
    while (true) {
        std::cout << "Register a new player? [y/n]: ";
        char regChoice;
        std::cin >> regChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (regChoice == 'y' || regChoice == 'Y') { new_player_reg(); std::cout << "\n"; }
        else break;
    }
    std::cout << "Player registration completed.\n\n";
    
    // =================================
    // |   3. Login existing players   |
    // =================================
    std::cout << "Part 3: Player login\nLogging in 4 players.\n\n";

    std::vector<int> DB_ids(4);
    std::vector<std::string> usernames(4);
    for (int i = 0; i < 4; ++i) {
        std::cout << "Player " << i + 1 << " login:\n";
        if (!player_login(DB_ids[i], usernames[i])) {
            std::cerr << "Login failed for Player " << i + 1 << ". Exiting.\n";
            close_DB_connection();
            return 1;
        }
        std::cout << "\n";
    }
    std::cout << "All players logged in successfully.\n\n";

    // =============================
    // |   4. Create a mock game   |
    // =============================
    std::cout << "Part 4: Create a mock game\n\n";
    Game game;
    game.startGame();

    auto& players = game.getPlayers();
    if (players.size() != 4) {
        std::cerr << "Error: Expected 4 players, got " << players.size() << "\n";
        close_DB_connection();
        return 1;
    }

    // Bind DB ids to players
    std::cout << "Binding DB ids to players...\n";
    for (int i = 0; i < 4; ++i) {
        players[i]->setDBId(DB_ids[i]);
    }

    int PlayCount = 1;
    while (!game.isGameOver()) {
        std::cout << "\n--- Play count: " << PlayCount++ << " ---\n";
        game.displayGameState();
        game.takeTurn();
    }

    int winner = game.getWinner();
    if (winner < 0) {
        std::cerr << "Error: Invalid winner index.\n";
        close_DB_connection();
        return 1;
    }

    std::cout << "\nGame over! Winner is Player " << winner + 1 << ".\n";

    // ============================
    // |   5. Save game results   |
    // ============================
    std::cout << "Part 5: Save game results\n\n";
    std::cout << "Saving game results to database...\n";
    if (!save_result_to_DB(game)) {
        std::cerr << "Failed to save game results to database.\n";
        close_DB_connection();
        return 1;
    }

    // ==============================
    // |   6. Leaderboard display   |
    // ==============================
    std::cout << "Part 6: Leaderboard display\n\n";
    display_leaderboard(10);
    std::cout << "\n";

    // ==============================
    // |   7. Close DB connection   |
    // ==============================
    std::cout << "Part 7: Close DB connection\n";
    close_DB_connection();

    return 0;
}

/*
    Compiling command:
    g++ 01.cpp card.cpp hand.cpp deck.cpp combination.cpp game.cpp player.cpp database.cpp -o 01.exe -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -I"C:\Program Files\OpenSSL-Win64\include" -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -L"C:\Program Files\OpenSSL-Win64\lib" -lmysql -lcrypto
*/
