#include "wormhole_nav/db_interface.hpp"
#include <sqlite3.h>
#include <iostream>

DBInterface::DBInterface(const std::string &db_path):db_path_(db_path),db_(nullptr) {}

bool DBInterface::open(){
    if(sqlite3_open(db_path_.c_str(), reinterpret_cast<sqlite3**>(&db_)) != SQLITE_OK){
        std::cerr << "Cannot open database: " << sqlite3_errmsg(reinterpret_cast<sqlite3*>(&db_)) << std::endl;
        return false;
    }
    return true;
}

void DBInterface::close(){
    if(db_) sqlite3_close(reinterpret_cast<sqlite3*>(&db_));
    db_ = nullptr;
}

std::vector<Wormhole> DBInterface::getWormholes(const std::string &map_from, const std::string &map_to){
    std::vector<Wormhole> wormholes;
    if(!db_) return wormholes;

    const char* sql = "SELECT id, map_from, map_to, x, y, yaw, overlap_rad, entry_x, entry_y, entry_yaw FROM wormholes WHERE map_from = ? AND map_to = ?;";
    sqlite3_stmt* stmt=nullptr;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(&db_), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, map_from.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, map_to.c_str(), -1, SQLITE_TRANSIENT);

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