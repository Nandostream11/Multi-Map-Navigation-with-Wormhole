#include "wormhole_nav/db_interface.hpp"
#include <sqlite3.h>
#include <iostream>
#include <rclcpp/rclcpp.hpp>

DBInterface::DBInterface(const std::string &db_path):db_path_(db_path),db_(nullptr) {}

DBInterface::~DBInterface() {
    close();
}

bool DBInterface::open(){
    if(sqlite3_open(db_path_.c_str(), reinterpret_cast<sqlite3**>(&db_)) != SQLITE_OK){
        RCLCPP_ERROR(rclcpp::get_logger("db_interface"), "Cannot open database: %s",  sqlite3_errmsg(reinterpret_cast<sqlite3*>(db_)));
        return false;
    }

        
    // Create table if it doesn't exist
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS wormholes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "map_from TEXT NOT NULL,"
        "map_to TEXT NOT NULL,"
        "x REAL NOT NULL,"
        "y REAL NOT NULL,"
        "yaw REAL NOT NULL,"
        "overlap_rad REAL DEFAULT 1.0,"
        "entry_x REAL,"
        "entry_y REAL,"
        "entry_yaw REAL);";

    char* err_msg = nullptr;

    if (sqlite3_exec(reinterpret_cast<sqlite3*>(db_), create_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        RCLCPP_ERROR(rclcpp::get_logger("db_interface"), "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    RCLCPP_INFO(rclcpp::get_logger("db_interface"), "Database opened successfully: %s", db_path_.c_str());
    return true;
}

void DBInterface::close(){
    if(db_){ 
        sqlite3_close(reinterpret_cast<sqlite3*>(db_));
        db_ = nullptr;
    }
}

std::vector<Wormhole> DBInterface::getWormholes(const std::string &map_from, const std::string &map_to){
    std::vector<Wormhole> wormholes;
    if(!db_){
        RCLCPP_ERROR(rclcpp::get_logger("db_interface"), "Database not open");
        return wormholes;
    }

    const char* sql = "SELECT id, map_from, map_to, x, y, yaw, overlap_rad, entry_x, entry_y, entry_yaw FROM wormholes WHERE map_from = ? AND map_to = ?;";
    sqlite3_stmt* stmt=nullptr;
        
    if (sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db_),sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RCLCPP_ERROR(rclcpp::get_logger("db_interface"), "Failed to prepare statement: %s", sqlite3_errmsg(reinterpret_cast<sqlite3*>(db_)));
        return wormholes;
    }
    // sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(&db_), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, map_from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, map_to.c_str(), -1, SQLITE_STATIC);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        Wormhole wh;
        wh.id = sqlite3_column_int(stmt, 0);
        wh.map_from = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        wh.map_to = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        wh.x = sqlite3_column_double(stmt, 3);
        wh.y = sqlite3_column_double(stmt, 4);
        wh.yaw = sqlite3_column_double(stmt, 5);
        wh.overlap_rad = sqlite3_column_double(stmt, 6);
        
        if(sqlite3_column_type(stmt, 7) != SQLITE_NULL)
            wh.entry_x = sqlite3_column_double(stmt, 7);
        if(sqlite3_column_type(stmt, 8) != SQLITE_NULL)
            wh.entry_y = sqlite3_column_double(stmt, 8);
        if(sqlite3_column_type(stmt, 9) != SQLITE_NULL)
            wh.entry_yaw = sqlite3_column_double(stmt, 9);

        wormholes.push_back(wh);
    }

    sqlite3_finalize(stmt);
    return wormholes;
}