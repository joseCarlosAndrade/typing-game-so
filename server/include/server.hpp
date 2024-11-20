#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <bits/stdc++.h>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

class Server {
private:
    int PORT;                    // Port number for the server
    int serverSocket;            // Socket descriptor for the server
    std::atomic<bool> isRunning; // Flag to indicate if the server is running

    std::map<int, int> rankings; // Player ID to Score map
    std::mutex rankingsMutex;    // Mutex to protect rankings

    std::vector<std::thread> threads; // Threads for handling clients

    void handlePlayer(int clientSocket, int playerId); // Handle an individual player
    void updateRanking(int playerId, int score);       // Update a player's ranking
    void closeAllThreads();                            // Clean up threads
    void closeSocket();                                // Close the server socket

public:
    explicit Server(int portno = 12345); // Constructor with default port
    ~Server();                           // Destructor to clean up resources
    void start();                        // Start the server
    void stop();                         // Stop the server
};

#endif
