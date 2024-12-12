#include "server.hpp"
#include "protocol.hpp"

// Constructor: Initialize server with a specific port
Server::Server(int portno) : PORT(portno), serverSocket(-1), gameState(ACCEPTING_CONNECTIONS), isRunning(false) {}

// Destructor: Clean up resources
Server::~Server() {
    stop();
}

// Start the command reading thread
void Server::startCommandThread() {
    threads.emplace_back(&Server::readCommands, this);
}


// Read commands through stdin to dictate the server phases, ran as thread
void Server::readCommands() {
    std::string command;
    while (true) {
        std::cin >> command;
        if (command == "stop") {
            setGameState(ENDGAME);
            break;
        } else if (command == "print") {
            printRankings();
        } else if (command == "phrase") {
            setGameState(WAITING_FOR_PLAYERS);
            std::cout << "Phrase sent. Waiting for players." << std::endl;
        } else if (command == "start") {
            setGameState(GAME_IN_PROGRESS);
            std::cout << "Game started." << std::endl;
        } else {
            std::cout << "Invalid command." << std::endl;
        }
    }
}


// thread safe function to change the game state
void Server::setGameState(STATES state){
    std::lock_guard<std::mutex> lock(gameStateMutex);
    gameState = state;

    // This switch triggers the sending of the phrase to all players
    if (state ==  STATES::WAITING_FOR_PLAYERS) {
        sendPhrase();
    }
    // This switch triggers the start message to all players
    if (state ==  STATES::GAME_IN_PROGRESS) {
        sendStart();
    }
    // This switch triggers the endgame message to all players and shutdown of the server
    if (state == STATES::ENDGAME) {
        sendEndgame();
        stop();
    }

}

// Start the server
void Server::start() {

    // Opens the socket that will listen to client connections

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

    // From this point the server is up and running, accetping connections from clients

    int playerId = 0;


    // Client accepting main loop
    while (isRunning && (gameState == ACCEPTING_CONNECTIONS)) {
        // Accepts client
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (!isRunning) break; // Stop accepting if server is stopping
            perror("Accept timeout");
            continue;
        }

        // Player data initialization in internal data structures
        initPlayerData(playerId);

        /*
            The server keeps 2 sockets per client, 1 to send data and one to receive

            The following section creates both sockets and initiates the threads
            to handle a client
        */

        // Creates the socket the server will use to send data to the client
        int senderSocket = getPlayerSocket(clientSocket, playerId, clientAddr);

        std::cout << "Player " << playerId << " connected." << std::endl;
        playerCount++; // Increments the number of players

        // Spawn a thread to handle the incoming player data
        threads.emplace_back(&Server::receivePlayerData, this, clientSocket, playerId);

        // Spawn a thread to send data to the player
        threads.emplace_back(&Server::sendPlayerData, this, senderSocket, playerId);

        // Stores the sockets of a player
        playerConections[playerId] = {clientSocket, senderSocket};

        // Increments the playerId
        playerId++;
    }

    // Close resources
    closeAllThreads();
    closeSocket();
}

// Run as a thread to check if the game is finished
void Server::waitForEndgame() {
    std::cout << "Wait For Endgame Started\n";
    std::unique_lock<std::mutex> lck(completionTimesMutex);
    // Sleeps the thread when its not finished, avoids busy waiting
    completionCV.wait(lck, [this] {return this->playerCount == this->finishedPlayers;});
    std::cout << "Endgame sequence started\n";
    // Sets the game as finished
    setGameState(STATES::ENDGAME);
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


// Handles incoming client messsages
void Server::receivePlayerData(int clientSocket, int player_id) {
    // Stores received messages
    char buffer[1024];
    while (isRunning) {
        
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        // If receiveing fails, player is treated as disconnected
        if (bytesReceived <= 0) {
            std::cout << "Player " << player_id << " disconnected." << std::endl;
            playerCount--;
            playerConections.erase(playerConections.find(player_id));
            close(clientSocket);
            return;
        }
        // Decodes message
        ClientMessage message = ClientMessage::decode(std::string(buffer));

        switch (message.type) {
            // Reccords player as ready for game start
            case ClientMessage::ClientMessageType::READY :
                storePlayerReady(player_id);
                break;
            // Updates ranking based in player new score
            case ClientMessage::ClientMessageType::PROGRESS :
                updateRanking(message.data.name, message.data.score, message.data.timestamp);
                break;
            // Records the completion time of a player
            case ClientMessage::ClientMessageType::DONE :
                storePlayerDone(player_id, message.data.timestamp);
                break;
        default:
            break;
        }
    }
    // Closes resources
    close(clientSocket);
    
}

// Ran as thread, handles sending data to players
void Server::sendPlayerData(int clientSocket, int player_id) {
    while (isRunning) {
        // Only state where messages are sent continuoslly
        if (gameState == STATES::GAME_IN_PROGRESS){
            // Sends the current ranking to player
            sendRankings(clientSocket, player_id);
            // Adds delay sot not to clog the system when multiple interfaces and server are all running on the same computer  
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
            
    }
    // Closes resources
    close(clientSocket);
}

// Encapsulates the action of sending ranking messages
void Server::sendRankings(int clientSocket, int player_id) {
    // Access the rankings
    std::unique_lock<std::mutex> rankLock(rankingsMutex);
    // Builds the message to be sent
    ServerMessage message;
    message.playerCount = playerCount;
    message.rankings = rankings;
    message.type = ServerMessage::ServerMessageType::RANKING;
    std::string str_message = message.encode();
    // Sends the message
    if (send(clientSocket, str_message.c_str(), str_message.size(), 0) < 0) {
        std::cerr << "ERROR sending rankings to player " << player_id << std::endl;
    }
}

// Initialize player data on maps so not to cause crashes, only in use fo playerReady as of now, but may be expanded
void Server::initPlayerData(int player_id) {
    std::lock_guard<std::mutex> lck(playerReadyMutex);
    playerReady.insert({player_id, false});
}


// Thread safely stores player finish timestamp
void Server::storePlayerDone(int player_id, int timestamp) {
    std::lock_guard<std::mutex> lck(completionTimesMutex);
    completionTimes.insert({player_id, timestamp});
    finishedPlayers++;
}


// Thread safely stores ready status
void Server::storePlayerReady(int player_id){
    std::lock_guard<std::mutex> lck(playerReadyMutex);
    playerReady[player_id] = true;
}


// Handles logic to get a socket to send player data, returns -1 on fail
int Server::getPlayerSocket(int clientSocket, int playerId, sockaddr_in addr) {

    // Port logic
    int newPort = 12345 + playerId + 1;

    // send new port to client
    std::string newPortStr = std::to_string(newPort);
    if (send(clientSocket, newPortStr.c_str(), newPortStr.size(), 0) < 0) {
        std::cerr << "Error sending new port to client.\n";
        return -1;
    } 

    // Creates the socket to handle connection
    int senderSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (senderSocket < 0) {
        perror("ERROR creating socket");
        return -1;
    }
   
    addr.sin_port = htons(newPort); 
    socklen_t len = sizeof(addr);
    std::cout << "Attempting connection to " << addr.sin_addr.s_addr << ":" << ntohs(addr.sin_port) << std::endl;
    
    // 3 attempts to connect
    int attempts = 3;
    bool success = false;
    while (attempts-- > 0) {
        if(connect(senderSocket, (sockaddr *)&addr, len) < 0 ){
            perror("ERROR connecting to client");
            // sleep for a bit
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else {
            success = true;
            break;
        }
    }
    
    if (!success) {
        std::cerr << "Failed to connect to client " << playerId << std::endl;
        return -1 ;
    }

    // Returns the new socket
    return senderSocket;
}

// Sends phrase message to all players
void Server::sendPhrase() {
    // Message building
    std::string message =  ServerMessage(playerCount, "testphrase", ServerMessage::ServerMessageType::PHRASE).encode();
    // Iterates through connections and sends to all players
    for (auto player_conn : playerConections) {
        if(send(player_conn.second.second, message.c_str(), message.size(), 0) < 0)
            std::cerr << "Error sending phrase to client.\n";
        else
            std::cout << "Phrase sent to " << player_conn.first << std::endl;
    }

}

// Sends start message to all players
void Server::sendStart() {
    // Message building
    std::string message =  ServerMessage(playerCount, ServerMessage::ServerMessageType::START).encode();
    // Iterates through connections and sends to all players
    for (auto player_conn : playerConections) {
        if(send(player_conn.second.second, message.c_str(), message.size(), 0) < 0)
            std::cerr << "Error sending start to client.\n";
        else
            std::cout << "Start sent to " << player_conn.first << std::endl;
    }

}

// Sends start message to all players
void Server::sendEndgame() {
    // Thread safely builds the message
    std::unique_lock<std::mutex> lck(rankingsMutex);
    ServerMessage message =  ServerMessage(playerCount, ServerMessage::ServerMessageType::END_GAME);
    message.rankings = rankings;
    std::string str_m = message.encode();
    // Iterates through connections and sends to all players
    for (auto player_conn : playerConections) {
        if(send(player_conn.second.second, str_m.c_str(), str_m.size(), 0) < 0)
            std::cerr << "Error sending endgame to client.\n";
        else
            std::cout << "Endgame sent to " << player_conn.first << std::endl;
    }
}


// Update rankings thread-safely
void Server::updateRanking(std::string playerName, int score, int timestamp) {
    std::lock_guard<std::mutex> lock(rankingsMutex);

    // Remove old score from the multimap
    auto it = playerScores.find(playerName);
    if (it != playerScores.end()) {
        int oldScore = it->second;
        for (auto itr = rankings.begin(); itr != rankings.end(); ++itr) {
            if (itr->second.first == oldScore && itr->first == playerName) {
                rankings.erase(itr);
                break;
            }
        }
    }

    // Update the player's score in the playerScores map
    playerScores[playerName] = score;

    // Insert the updated score into the multimap
    rankings.insert({playerName, {score, timestamp}});
}

// Debug function, prints rankings
void Server::printRankings() {
    std::lock_guard<std::mutex> lock(rankingsMutex);
    std::cout << "Rankings:" << std::endl;
    for (auto it = rankings.rbegin(); it != rankings.rend(); ++it) { // Iterate in descending order
        std::cout << it->first << ": " << it->second.first << ", " << std::endl;
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
