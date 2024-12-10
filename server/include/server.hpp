#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <bits/stdc++.h>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

enum STATES {
    ACCEPTING_CONNECTIONS, // accepts new connections
    WAITING_FOR_PLAYERS, // waits for players to confirm they are ready 
    GAME_IN_PROGRESS, // game is in progress
    ENDGAME // game has ended, show ranks
};

class Server {
private:
    int PORT;                    // Port number for the server
    int serverSocket;            // Socket descriptor for the server

    STATES gameState; 

    std::multimap<std::string, std::pair<int, int>> rankings; // {{score, timestamp}, name}
    std::map<std::string, int> playerScores;  // Player name to score map for quick lookups
    std::mutex rankingsMutex;              // Mutex to protect rankings

    std::mutex gameStateMutex;

    std::map<int, std::pair<int, int>> playerConections; //PlayerID -> player socket{sender, receiver}

    std::vector<std::thread> threads; // Threads for handling clients

    std::thread mainThread; // main thread to handle server functions

    void handlePlayer(int clientSocket, int playerId, sockaddr_in addr);     // Handle an individual player
    void updateRanking(std::string playerName, int score, int timestamp); // Update a player's ranking
    void printRankings();
    void closeAllThreads(); // Clean up threads
    void closeSocket();     // Close the server socket


public:
    explicit Server(int portno = 12345); // Constructor with default port
    ~Server();                           // Destructor to clean up resources
    void startCommandThread();
    void readCommands();
    void start();                        // Start the server
    void stop();                         // Stop the server
    void setGameState(STATES state);
    std::atomic<bool> isRunning; // Flag to indicate if the server is running

};

#endif
