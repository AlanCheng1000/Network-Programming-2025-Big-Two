#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "card.h"
#include "player.h"
#include "game.h"

// Connection config
bool connect_to_DB(const std::string& host,
                   const std::string& user,
                   const std::string& password,
                   const std::string& dbname,
                   unsigned int port = 3306);

// Close connection
void close_DB_connection();

// Register a new player
bool is_valid_pwd(const std::string& pwd);
void new_player_reg();

// Login existing player
bool player_login(int& db_id, std::string& username_out);

// Score update
bool update_player_score(const std::string& player_name, int score_delta);

// Display the leaderboard
void display_leaderboard(int top_n);

// Generate hash password
std::string hash_password(const std::string& password);

// Additional database functions
bool save_result_to_DB(const Game& game);

// Take AI Agent from DB
bool acquire_AI_Agent_From_DB(int needed, std::vector<int>& AI_ids, std::vector<std::string>& AI_names);

// Release AI Agent to DB
bool release_Agents_To_DB(const std::vector<int>& ids);

// Set all players to be online
bool set_all_players_online(const std::vector<int>& db_ids, bool is_online);

// Set specified players to be online/offline
bool set_players_online_status(int db_id, bool is_online);
#endif // DATABASE_H