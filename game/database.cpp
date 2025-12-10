#define NOMINMAX
#include "database.h"
#include <mysql.h>
#include <iostream>
#include <iomanip>
#include <openssl/sha.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <regex>

static MYSQL* conn = nullptr;

static int penalty_calc(const std::vector<Card>& hand) {
    int number_of_cards = static_cast<int>(hand.size());
    int penalty = 0;
    
    int countTwos = 0;
    for (const auto& card : hand) {
        if (card.getrank() == Rank::Two) countTwos++;
    }
    penalty += number_of_cards * std::max(countTwos, 1);
    return penalty * (-1);
}

// Connection config
bool connect_to_DB(const std::string& host,
                   const std::string& user,
                   const std::string& password,
                   const std::string& dbname,
                   unsigned int port) {
    conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL initialization failed.\n";
        return false;
    }

    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                            dbname.c_str(), port, nullptr, 0)) {
        std::cerr << "Connection error: " << mysql_error(conn) << "\n";
        return false;
    }

    std::cout << "Connected successfully to " << dbname << ".\n";
    return true;
}

// Close connection
void close_DB_connection() {
    if (conn) {
        mysql_close(conn);
        conn = nullptr;
        std::cout << "Connection closed.\n";
    }
}

// Register a new player
bool is_valid_pwd(const std::string& pwd) {
    std::regex pattern(R"(^\d{4}[A-Za-z]{4})");
    return std::regex_match(pwd, pattern);
}

void new_player_reg() {
    std::string username, pwd, confirm_pwd;
    std::cout << "Please enter your username: ";
    std::getline(std::cin, username);

    while (true) {
        std::cout << "Now, please create your own password.\n";
        std::cout << "It should consist of four digits and be followed by four letters.\n";
        std::cout << "For example: 6539tBkQ\n";
        std::cout << "[CAUTION] Passwords are case-sensitive.\n";
        std::cout << "Enter password: ";
        std::getline(std::cin, pwd);
        if (!is_valid_pwd(pwd)) {
            std::cout << "Invalid password format. Try again.\n";
            continue;
        }
        std::cout << "Please confirm your password: ";
        std::getline(std::cin, confirm_pwd);
        if (pwd == confirm_pwd) break;
        else std::cout << "Passwords do not match! Please try again.\n";
    }

    std::string password_hash = hash_password(pwd);
    if (!conn) return;
    std::string query = "INSERT INTO players (bot, name, password_hash, total_score, wins, losses) VALUES (0, '" +
                        username + "', '" + password_hash + "', 1000, 0, 0);";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Registration error: " << mysql_error(conn) << "\n";
    } else {
        std::cout << "Profile successfully configured.\n";
        std::cout << "Welcome, " << username << "!\n";
        std::cout << "You can now log in and start playing the game.\n";
        std::cout << "Your initial score is set to 1000.\n\n";
    }

    std::string select_query = "SELECT id, name, total_score, wins, losses FROM players WHERE name = '" + username + "';";
    if (mysql_query(conn, select_query.c_str())) {
        std::cerr << "Profile fetching error: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;

    std::cout << "Your profile:\n";
    std::cout << "-----------------------\n";
    while ((row = mysql_fetch_row(res))) {
        std::cout << "ID: " << row[0] << "\n"
                  << "Name: " << row[1] << "\n"
                  << "Total Score: " << row[2] << "\n"
                  << "Wins: " << row[3] << "\n"
                  << "Losses: " << row[4] << "\n";
    }
    std::cout << "-----------------------\n\n";

    mysql_free_result(res);
}

// Login existing player
bool player_login(int& db_id, std::string& username_out) {
    if (!conn) return false;

    std::string username, pwd;
    while (true) {
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Enter password: ";
        std::getline(std::cin, pwd);

        std::string password_hashed = hash_password(pwd);
        std::string query = "SELECT id, name FROM players WHERE name = '" + username +
                            "' AND password_hash = '" + password_hashed + "' AND bot = 0 AND in_game = 0;";

        if (mysql_query(conn, query.c_str())) {
            std::cerr << "Login error: " << mysql_error(conn) << "\n";
            return false;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);

        if (row) {
            db_id = std::stoi(row[0]);
            username_out = row[1];
            mysql_free_result(res);

            std::string updatequery = "UPDATE players SET in_game = 1, last_login = NOW() WHERE id = " + std::to_string(db_id) + ";";

            if (mysql_query(conn, updatequery.c_str())) {
                std::cerr << "Update last login error: " << mysql_error(conn) << "\n";
            }

            std::cout << "Login successful. Welcome back, " << username_out << "!\n";
            return true;
        } else {
            std::cout << "Invalid username or password. Please try again.\n";
            mysql_free_result(res);
        }
    }
}

// Score update
bool update_player_score(const std::string& player_name, int score_delta) {
    if (!conn) return false;

    std::string query = "UPDATE players SET total_score = total_score + " + std::to_string(score_delta) +
                        " WHERE name = '" + player_name + "';";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Update error: " << mysql_error(conn) << "\n";
        return false;
    }

    std::cout << "Player " << player_name << "'s score updated by " << (score_delta >= 0 ? "+" : "") << score_delta << ".\n";
    return true;
}

// Display the leaderboard
void display_leaderboard(int top_n) {
    if (!conn) return;

    std::string query = "SELECT name, total_score, wins, losses FROM players WHERE bot = 0 ORDER BY total_score DESC LIMIT " + std::to_string(top_n) + ";";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query error: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Store result error: " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_ROW row;
    std::cout << "\n";
    std::cout << "================ Leaderboard ================\n";
    std::cout << std::left << std::setw(19) << "Player_Name"
              << std::setw(10) << "Score"
              << std::setw(10) << "Wins"
              << std::setw(10) << "Losses" << "\n";
    std::cout << "=============================================\n";

    while ((row = mysql_fetch_row(res))) {
        std::cout << std::left << std::setw(19) << row[0]
                  << std::setw(10) << row[1]
                  << std::setw(10) << row[2]
                  << std::setw(10) << row[3] << "\n";
    }

    mysql_free_result(res);
    std::cout << "=============================================\n";
}

// Generate hash password
std::string hash_password(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    std::stringstream ss;
    for (unsigned char byte : hash) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

// Persist game result to database
bool save_result_to_DB(const Game& game) {
    if (!conn) {
        std::cerr << "No active database connection.\n";
        return false;
    }

    if (!game.isGameOver()) {
        std::cerr << "Game is not over. Cannot save results.\n";
        return false;
    }

    int winnerSeat = game.getWinner();
    if (winnerSeat < 0) {
        std::cerr << "Invalid winner seat index.\n";
        return false;
    }

    auto& players = const_cast<Game&>(game).getPlayers();
    const int playerCount = static_cast<int>(players.size());
    if (winnerSeat >= playerCount) {
        std::cerr << "Winner seat index out of range.\n";
        return false;
    }

    // Check if all players are bound to a database ID
    for (int i = 0; i < playerCount; ++i) {
        int db_id = players[i]->getDBId();
        if (db_id < 0) {
            std::cerr << "Player at seat " << i << " is not bound to a database ID. Aborting save.\n";
            return false;
        }
    }

    // Prepare and execute score updates
    std::vector<int> deltas(playerCount, 0);
    int totalPenalty = 0;

    for (int i = 0; i < playerCount; ++i) {
        if (i == winnerSeat) continue;
        const auto& handCards = players[i]->getHand().getCards();
        int penalty = penalty_calc(handCards);
        deltas[i] = penalty;
        totalPenalty += penalty;
    }

    deltas[winnerSeat] = totalPenalty * -1;
    int winnerDBId = players[winnerSeat]->getDBId();

    // Start transaction
    if (mysql_query(conn, "START TRANSACTION;")) {
        std::cerr << "Transaction start error: " << mysql_error(conn) << "\n";
        return false;
    }

    // Insert game record
    std::stringstream matchQuery;
    matchQuery << "INSERT INTO matches (start_time, end_time, winner_id) VALUES (CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, " << winnerDBId << ");";
    if (mysql_query(conn, matchQuery.str().c_str())) {
        std::cerr << "Match insert error: " << mysql_error(conn) << "\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    long long match_id = mysql_insert_id(conn);

    // Insert player results and update player table
    for (int i = 0; i < playerCount; ++i) {
        int db_id = players[i]->getDBId();
        int seat_index = players[i]->getseatIndex();
        int score_delta = deltas[i];
        bool is_winner = (i == winnerSeat);

        std::stringstream resultQuery;
        resultQuery << "INSERT INTO match_players (match_id, player_id, seat_index, score_change, is_winner) VALUES ("
                    << match_id << ", " << db_id << ", " << seat_index + 1 << ", " << score_delta << ", " << (is_winner ? 1 : 0) << ");";
        if (mysql_query(conn, resultQuery.str().c_str())) {
            std::cerr << "Match result insert error for player " << db_id << ": " << mysql_error(conn) << "\n";
            mysql_query(conn, "ROLLBACK;");
            return false;
        }

        std::stringstream updatePlayerQuery;
        updatePlayerQuery << "UPDATE players SET total_score = total_score + " << score_delta;
        if (is_winner) {
            updatePlayerQuery << ", wins = wins + 1 ";
        } else {
            updatePlayerQuery << ", losses = losses + 1 ";
        }
        updatePlayerQuery << " WHERE id = " << db_id << ";";
        if (mysql_query(conn, updatePlayerQuery.str().c_str())) {
            std::cerr << "Player update error for player " << db_id << ": " << mysql_error(conn) << "\n";
            mysql_query(conn, "ROLLBACK;");
            return false;
        }
    }

    // Commit transaction
    if (mysql_query(conn, "COMMIT;")) {
        std::cerr << "Transaction commit error: " << mysql_error(conn) << "\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    std::cout << "Game results successfully saved to the database.\n";
    return true;
}

bool set_specified_player_online_status(int db_id, bool is_online) {
    if (!conn) {
        std::cerr << "No active database connection.\n";
        return false;
    }
    if (db_id < 0) return false;

    std::string query = "UPDATE players SET is_online = " + std::to_string(is_online ? 1 : 0) +
                        " WHERE id = " + std::to_string(db_id) + ";";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Update online status error: " << mysql_error(conn) << "\n";
        return false;
    }

    return true;
}

// Set all players to be online
bool set_all_players_online(const std::vector<int>& db_ids, bool is_online) {
    if (!conn) {
        std::cerr << "No active database connection.\n";
        return false;
    }

    if (db_ids.empty()) return true;

    std::stringstream updateQuery;
    updateQuery << "UPDATE players SET is_online = " << (is_online ? 1 : 0) << " WHERE id IN (";
    bool first = true;
    for (int id : db_ids) {
        if (id < 0) continue;
        if (!first) updateQuery << ", ";
        updateQuery << id;
        first = false;
    }
    updateQuery << ");";

    if (first) return true; // No valid IDs to update

    if (mysql_query(conn, updateQuery.str().c_str())) {
        std::cerr << "Update online status error: " << mysql_error(conn) << "\n";
        return false;
    }

    return true;
}

// Take AI Agent from DB
bool acquire_AI_Agent_From_DB(int needed, std::vector<int>& AI_ids, std::vector<std::string>& AI_names) {
    if (!conn) {
        std::cerr << "No active database connection.\n";
        return false;
    }

    if (needed <= 0) {
        AI_ids.clear();
        AI_names.clear();
        return true;
    }

    AI_ids.clear();
    AI_names.clear();

    if (mysql_query(conn, "START TRANSACTION;")) {
        std::cerr << "Transaction start error: " << mysql_error(conn) << "\n";
        return false;
    }

    std::string query = "SELECT id, name FROM players WHERE bot = 1 AND in_game = 0 LIMIT " + std::to_string(needed) + " FOR UPDATE;";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "AI selection error: " << mysql_error(conn) << "\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Store result error: " << mysql_error(conn) << "\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        AI_ids.push_back(std::stoi(row[0]));
        AI_names.push_back(row[1]);
    }

    mysql_free_result(res);

    if (static_cast<int>(AI_ids.size()) < needed) {
        std::cerr << "Not enough available AI agents.\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    std::stringstream updateQuery;
    updateQuery << "UPDATE players SET in_game = 1 WHERE id IN (";
    for (size_t i = 0; i < AI_ids.size(); ++i) {
        if (i > 0) updateQuery << ", ";
        updateQuery << AI_ids[i];
    }
    updateQuery << ");";

    if (mysql_query(conn, updateQuery.str().c_str())) {
        std::cerr << "AI update error: " << mysql_error(conn) << "\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    if (mysql_query(conn, "COMMIT;")) {
        std::cerr << "Transaction commit error: " << mysql_error(conn) << "\n";
        mysql_query(conn, "ROLLBACK;");
        return false;
    }

    return true;
}

// Release AI Agent to DB
bool release_Agents_To_DB(const std::vector<int>& ids) {
    if (!conn) {
        std::cerr << "No active database connection.\n";
        return false;
    }

    if (ids.empty()) return true;

    std::stringstream updateQuery;
    updateQuery << "UPDATE players SET in_game = 0 WHERE id IN (";
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) updateQuery << ", ";
        updateQuery << ids[i];
    }
    updateQuery << ");";

    if (mysql_query(conn, updateQuery.str().c_str())) {
        std::cerr << "AI release error: " << mysql_error(conn) << "\n";
        return false;
    }

    return true;
}