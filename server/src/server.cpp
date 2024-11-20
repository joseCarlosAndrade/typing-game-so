#include "server.hpp"

// Constructor: Initialize server with a specific port
Server::Server(int portno) : PORT(portno), serverSocket(-1), isRunning(false) {}

// Destructor: Clean up resources
Server::~Server() {
    stop();
}

// Start the server
void Server::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("ERROR opening socket");
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR on binding");
        return;
    }

    if (listen(serverSocket, 5) < 0) {
        perror("ERROR on listening");
        return;
    }

    std::cout << "Server listening on port " << PORT << std::endl;
    isRunning = true;

    int playerId = 0;

    while (isRunning) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);

        if (clientSocket < 0) {
            if (!isRunning) break; // Stop accepting if server is stopping
            perror("ERROR on accept");
            continue;
        }

        std::cout << "Player " << playerId << " connected." << std::endl;

        // Spawn a thread to handle the player
        threads.emplace_back(&Server::handlePlayer, this, clientSocket, playerId++);
    }

    closeAllThreads();
    closeSocket();
}

// Stop the server gracefully
void Server::stop() {
    if (isRunning) {
        isRunning = false;
        closeSocket();
        closeAllThreads();
        std::cout << "Server stopped." << std::endl;
    }
}

// Handle a single player
void Server::handlePlayer(int clientSocket, int playerId) {
    char buffer[1024];

    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            std::cout << "Player " << playerId << " disconnected." << std::endl;
            close(clientSocket);
            return;
        }

        // Process player's input
        std::string input(buffer);
        int score = input.length(); // Example: Score based on input length

        // Update rankings safely
        updateRanking(playerId, score);
    }

    close(clientSocket);
}

// Update rankings thread-safely
void Server::updateRanking(int playerId, int score) {
    std::lock_guard<std::mutex> lock(rankingsMutex);
    rankings[playerId] += score;
    std::cout << "Player " << playerId << "'s score updated to " << rankings[playerId] << std::endl;
}

// Close all threads
void Server::closeAllThreads() {
    for (auto &thread : threads) {
        if (thread.joinable()) thread.join();
    }
    threads.clear();
}

// Close the server socket
void Server::closeSocket() {
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
}
