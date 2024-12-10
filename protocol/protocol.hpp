#ifndef PROTOCOL_H
#define PROTOCOL_C

#include <bits/stdc++.h>

// Struct for crucial data that will be passed
struct PlayerData {
    std::string name; // Player name
    int score;        // Player's score
    int timestamp;    // Timestamp

    // Constructor
    PlayerData(std::string name, int score, int timestamp)
        : name(name),
          score(score),
          timestamp(timestamp) {}
};

class ServerMessage {
public:
    int playerCount;
    std::multimap<std::string, std::pair<int, int>> rankings; // {{score, timestamp}, name}
    bool isRunning;

    // Constructor
    ServerMessage(int count, std::multimap<std::string, std::pair<int, int>> rankings)
        : playerCount(count),
          rankings(rankings),
          isRunning(true) {}

    ServerMessage(int count)
        : playerCount(count),
          rankings(),
          isRunning(true) {}

    // Encoding function
    std::string encode();

    // Decoding function
    static ServerMessage decode(const std::string &data);
};

class ClientMessage {
public:
    PlayerData data;

    // Constructor
    ClientMessage(std::string name, int sc, int ts = getCurrentTimestamp())
        : data(name, sc, ts) {}

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