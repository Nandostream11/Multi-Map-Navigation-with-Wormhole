#pragma once
#include <string>
#include <vector>
#include <optional>

struct Wormhole {
    int id;
    std::string map_from;
    std::string map_to;
    double x,y,yaw;
    double overlap_rad;
    std::optional<double> entry_x, entry_y, entry_yaw;
};

class DBInterface{
    public:
        explicit DBInterface(const std::string &db_path);
        ~DBInterface();  // destructor
        bool open();
        void close();
        std::vector<Wormhole> getWormholes(const std::string &map_fom, const std::string &map_to);
    private:
        std::string db_path_;
        void *db_; // SQLite3 DB pointer
};