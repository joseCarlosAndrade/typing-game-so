#ifndef PROTOCOL_H
#define PROTOCOL_C

#include <bits/stdc++.h>

// Struct for crucial data that will be passed
struct PlayerData {
    int id;        // Player ID
    int score;     // Player's score
    int timestamp; // Timestamp

    // Constructor
    PlayerData(int id, int score, int timestamp)
        : id(id),
          score(score),
          timestamp(timestamp) {}
};

// Struct made for comparing player data when Adding
struct ComparePlayerData {
    bool operator()(const PlayerData &a, const PlayerData &b) const {
        if (a.score != b.score) {
            return a.score > b.score; // Descending order by score
        }
        // Otherwise, use timestamp (earlier is better)
        return a.timestamp < b.timestamp;
    }
};

class ServerMessage {
public:
    int playerCount;
    std::set<PlayerData, ComparePlayerData> allData;
    bool isRunning;

    // Constructor
    ServerMessage(int count)
        : playerCount(count),
          allData(),
          isRunning(true) {}

    // Set player data
    void insert(int id, int score, int timestamp) {
        allData.insert(PlayerData(id, score, timestamp));
    }

    // Encoding function
    std::string encode();

    // Decoding function
    static ServerMessage decode(const std::string &data);
};

class ClientMessage {
public:
    PlayerData data;

    // Constructor
    ClientMessage(int id, int sc, int ts = getCurrentTimestamp())
        : data(id, sc, ts) {}

    // Encoding function
    std::string encode();

    // Decoding function
    static ClientMessage decode(const std::string &data);

    // Helper function to get current timestamp
    static int getCurrentTimestamp() {
        return static_cast<int>(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    }
};

#endif