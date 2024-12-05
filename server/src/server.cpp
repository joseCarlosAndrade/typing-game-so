#include "server.hpp"
#include "protocol.hpp"

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
        threads.emplace_back(&Server::handlePlayer, this, clientSocket, playerId++, clientAddr);
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
void Server::handlePlayer(int clientSocket, int playerId, sockaddr_in addr) {
    char buffer[1024];

    int senderSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (senderSocket < 0) {
        perror("ERROR creating socket");
        return;
    }
    // For now, the receiving port of the client is always the next from the sender
    // addr.sin_port = htons(ntohs(addr.sin_port) + 1); 
    addr.sin_port = htons(12346); // TODO : no more gambiarra, create a system for the second port
    socklen_t len = sizeof(addr);
    std::cout << "Attempting connection to " << addr.sin_addr.s_addr << ":" << ntohs(addr.sin_port) << std::endl;
    if(connect(senderSocket, (sockaddr *)&addr, len) < 0 ){
        perror("ERROR connecting to client");
        return;
    }

    // Saves to the class map
    playerConections[playerId] = {clientSocket, senderSocket};

    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));

        // constantly tries to receive data from client until it disconnects
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            std::cout << "Player " << playerId << " disconnected." << std::endl;
            close(clientSocket);
            return;
        }

        // Process player's input
        std::string input(buffer);
        ClientMessage message = ClientMessage::decode(input);
        std::cout << message.data.name << "[" << playerId << "]" << ": " << message.data.score << ", " << message.data.timestamp << std::endl;

        // Update rankings safely
        updateRanking(message.data.name, message.data.score, message.data.timestamp);

        // Send response to player
        ServerMessage response(rankings.size(), rankings);
        std::string encodedresponse = response.encode();
        if (send(senderSocket, encodedresponse.c_str(), encodedresponse.size(), 0) < 0) {
            std::cerr << "Error sending response to client.\n";
            break;
        } else
            std::cout << "Response sent to socket " << senderSocket << ": " << encodedresponse << std::endl;
    }

    close(clientSocket);
}

// Update rankings thread-safely
void Server::updateRanking(std::string playerName, int score, int timestamp) {
    std::lock_guard<std::mutex> lock(rankingsMutex);

    // Remove old score from the multimap
    auto it = playerScores.find(playerName);
    if (it != playerScores.end()) {
        int oldScore = it->second;
        for (auto itr = rankings.begin(); itr != rankings.end(); ++itr) {
            if (itr->first.first == oldScore && itr->second == playerName) {
                rankings.erase(itr);
                break;
            }
        }
    }

    // Update the player's score in the playerScores map
    playerScores[playerName] = score;

    // Insert the updated score into the multimap
    rankings.insert({{score, timestamp}, playerName});
    std::cout << "Player " << playerName << "'s score updated to " << score << std::endl;
}

void Server::printRankings() {
    std::lock_guard<std::mutex> lock(rankingsMutex);
    std::cout << "Rankings:" << std::endl;
    for (auto it = rankings.rbegin(); it != rankings.rend(); ++it) { // Iterate in descending order
        std::cout << it->second << ": " << it->first.first << ", " << std::endl;
    }
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
