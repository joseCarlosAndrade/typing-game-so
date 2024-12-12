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

// Message sent by the server to the client
class ServerMessage {
public:
    // Types of message
    enum ServerMessageType {PHRASE =1, START, RANKING, END_GAME}; // 0, 1, 2, 3

    int playerCount; // Number of players connected
    // Stores the rankings
    std::multimap<std::string, std::pair<int, int>> rankings; // {{score, timestamp}, name}
    bool isRunning; // Running flag
    std::string phrase; // Target phrase
    ServerMessageType type; // Message type

    // Constructors
    ServerMessage(int count, ServerMessageType type)
        : playerCount(count),
          rankings(),
          isRunning(true),
          type(type) {}
    
    ServerMessage(int countm, std::string phrase, ServerMessageType type)
        : playerCount(countm),
          rankings(),
          isRunning(true),
          phrase(phrase),
          type(type) {}

    ServerMessage()
        : playerCount(-1),
        rankings(),
        isRunning(true) {}


    // Server messages are passed via socket as text strings, following encoding and deconding conventions


    // Encoding function
    std::string encode();

    // Decoding function
    static ServerMessage decode(const std::string &data);
};


// Message sent by the client, to the server
class ClientMessage {
public:
    // Types of messages
    enum ClientMessageType {READY, PROGRESS, DONE}; //0, 1 2

    PlayerData data; // Info about the player
    ClientMessageType type; // Message type

    // Constructor
    ClientMessage(std::string name, int sc, int ts = getCurrentTimestamp(), ClientMessageType type = ClientMessageType::PROGRESS)
        : data(name, sc, ts), type(type) {}

    ClientMessage(ClientMessageType type)
        : data("", 0, 0), type(type) {}

        
    // Client messages are passed via socket as text strings, following encoding and deconding conventions



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