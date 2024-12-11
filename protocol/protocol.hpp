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
    enum ServerMessageType {PHRASE, START, RANKING, END_GAME}; // 0, 1, 2, 3

    int playerCount;
    std::multimap<std::string, std::pair<int, int>> rankings; // {{score, timestamp}, name}
    bool isRunning;
    ServerMessageType type;
    std::string phrase;

    // P causa do -Werror tinha uns warning estanho, ai p compilar eu tirei esse constructor q so usava uma vez tbm
    // Constructor
    // ServerMessage(int count, std::multimap<std::string, std::pair<int, int>> rankings, std::string phrase, ServerMessageType type)
    //     : playerCount(count),
    //       rankings(rankings),
    //       isRunning(true),
    //       phrase(phrase),
    //       type(type) {}

    ServerMessage(int count, ServerMessageType type)
        : playerCount(count),
          rankings(),
          isRunning(true),
          type(type) {}

    ServerMessage()
        : playerCount(-1),
        rankings(),
        isRunning(true) {}

    // Encoding function
    std::string encode();

    // Decoding function
    static ServerMessage decode(const std::string &data);
};

class ClientMessage {
public:
    enum ClientMessageType {READY, PROGRESS, DONE}; //0, 1 2

    PlayerData data;
    ClientMessageType type;

    // Constructor
    ClientMessage(std::string name, int sc, int ts = getCurrentTimestamp(), ClientMessageType type = ClientMessageType::PROGRESS)
        : data(name, sc, ts), type(type) {}

    ClientMessage(ClientMessageType type)
        : data("", 0, 0), type(type) {}

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