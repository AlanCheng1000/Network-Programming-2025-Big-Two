#define NOMINMAX
#include "database.h"
#include "game.h"
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <set>
#include <random>
#include <algorithm>

int main() {
    // ==========================================================================
    // |   Please run the whole code at the same time.                          |
    // |   To save time, it is recommended to compile game.cpp with AIplayer.   |
    // ==========================================================================

    std::cout << "\nHello! This is the Big2 Game Module Test Program.\n\n";

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
    std::cout << "Part 3: Player login\nLogging in players.\n\n";

    int human_count = 0;
    
    do {
        std::cout << "Enter number of human players (1-4): ";
        std::cin >> human_count;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (human_count < 1 || human_count > 4) {
            std::cout << "Invalid input. Please enter a number between 1 and 4.\n";
        }
    } while (human_count < 1 || human_count > 4);

    int AI_count = 4 - human_count;

    std::vector<int> DB_ids(4, -1);
    std::vector<std::string> usernames(4);
    std::set<int> unique_ids;

    for (int i = 0; i < human_count; ++i) {
        while (true) {
            std::cout << "Player " << i + 1 << " login:\n";

            int db_id;
            std::string username;

            if (!player_login(db_id, username)) {
                std::cout << "Login failed. Try again.\n\n";
                continue;
            }

            if (unique_ids.count(db_id)) {
                std::cout << "This player is already logged in. Please use a different account.\n\n";
                continue;
            }

            unique_ids.insert(db_id);
            DB_ids[i] = db_id;
            usernames[i] = username;
            std::cout << "\n";
            break;
        }
    }

    std::vector<int> AI_ids;
    std::vector<std::string> AI_names;
    if (AI_count > 0) {
        if (!acquire_AI_Agent_From_DB(AI_count, AI_ids, AI_names)) {
            std::cerr << "Failed to acquire AI agents from database.\n";
            return 1;
        }

        for (int j = 0; j < AI_count; ++j) {
            int seat_index = human_count + j;
            DB_ids[seat_index] = AI_ids[j];
            usernames[seat_index] = AI_names[j];
            std::cout << "AI Player " << seat_index + 1 << " logged in as '" << AI_names[j] << "' (ID: " << AI_ids[j] << ").\n";
        }
        std::cout << "\n";
    }

    std::cout << "All players logged in successfully.\n\n";

    // =============================
    // |   4. Create a mock game   |
    // =============================
    std::cout << "Part 4: Create a mock game\n\n";
    Game game;

    std::vector<int> seatOrder = {0, 1, 2, 3};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(seatOrder.begin(), seatOrder.end(), g);

    std::vector<PlayerType> types(4);
    std::vector<int> DB_ids_shuffled(4);
    std::vector<std::string> usernames_shuffled(4);

    for (int i = 0; i < 4; ++i) {
        int originalIndex = seatOrder[i];
        DB_ids_shuffled[i] = DB_ids[originalIndex];
        usernames_shuffled[i] = usernames[originalIndex];

        if (originalIndex < human_count) {
            types[i] = PlayerType::Human;
        } else {
            types[i] = PlayerType::AI;
        }
    }

    DB_ids.swap(DB_ids_shuffled);
    usernames.swap(usernames_shuffled);

    for (int i = 0; i < 4; ++i) {
        std::cout << "Seat " << i + 1 << ": " << usernames[i]
                  << "\t(ID: " << DB_ids[i] << ", "
                  << (types[i] == PlayerType::Human ? "Human" : "AI") << ")\n";
    }
    std::cout << "\n";

    game.initializePlayers(types);
    game.setSeatDB_IDs(DB_ids);
    game.startGame();

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

    if (!AI_ids.empty()) {
        if (!release_Agents_To_DB(DB_ids)) {
            std::cerr << "Failed to release all agents to database.\n";
            close_DB_connection();
            return 1;
        }
    }

    // ============================
    // |   5. Save game results   |
    // ============================
    std::cout << "\nPart 5: Save game results\n\n";
    std::cout << "Saving game results to database...\n";
    if (!save_result_to_DB(game)) {
        std::cerr << "Failed to save game results to database.\n";
        close_DB_connection();
        return 1;
    }

    // ==============================
    // |   6. Leaderboard display   |
    // ==============================
    std::cout << "\nPart 6: Leaderboard display\n\n";
    display_leaderboard(8);
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