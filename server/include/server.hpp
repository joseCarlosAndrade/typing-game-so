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

    int playerCount = 0;
    std::string phrase = std::string("shogo eh bobalhao"); // Target string


    std::map<int, bool> playerReady; // Stores if a player has sent the ready message
    std::mutex playerReadyMutex; // playerReady mutex

    std::multimap<std::string, std::pair<int, int>> rankings; // {{score, timestamp}, name}
    std::map<std::string, int> playerScores;  // Player name to score map for quick lookups
    std::mutex rankingsMutex;              // Mutex to protect rankings

    std::mutex gameStateMutex;
    std::condition_variable gameStateCV; // Blocks treads in game states when its needed
    std::condition_variable messageSendingCV; // CV to avoid deadlocks in message sending

    // End game management
    std::map<int, int> completionTimes; // {id, last timestamps}
    std::mutex completionTimesMutex;
    int finishedPlayers = 0;
    std::condition_variable completionCV;

    std::map<int, std::pair<int, int>> playerConections; //PlayerID -> player socket{receiver, sender}

    std::vector<std::thread> threads; // Threads for handling clients
    

    std::thread mainThread; // main thread to handle server functions

    int getPlayerSocket(int clientSocket, int playerId, sockaddr_in addr);     // Handle an individual player
    void updateRanking(std::string playerName, int score, int timestamp); // Update a player's ranking
    void printRankings();
    void closeAllThreads(); // Clean up threads
    void closeSocket();     // Close the server socket
    void receivePlayerData(int clientSocket, int player_id); // Manages the receiving of a clients messages
    void sendPlayerData(int clientSocket, int player_id); // Mangaes sending messages to clients
    void storePlayerReady(int player_id); // Thread safely stores the ready state of a player
    void storePlayerDone(int player_id, int timestamp); // Thread safely stores the finished state of a player
    void initPlayerData(int player_id);  // Initializes some map records so not to cause crashes
    void sendRankings(int clientSocket, int player_id); // Wrapper to build the ranking message
    void sendStart(); // Sends start message to all players
    void sendEndgame(); // Sends endgame message to all players
    void waitForEndgame(); // Runs as thread a function to await for all players to send the done message

public:
    explicit Server(int portno = 12345); // Constructor with default port
    ~Server();                           // Destructor to clean up resources
    void startCommandThread();
    void readCommands();
    void start();                        // Start the server
    void stop();                         // Stop the server
    void setGameState(STATES state);
    void sendPhrase();
    std::atomic<bool> isRunning; // Flag to indicate if the server is running

};

#endif
