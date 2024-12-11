#include "server.hpp"
#include "protocol.hpp"

// Constructor: Initialize server with a specific port
Server::Server(int portno) : PORT(portno), serverSocket(-1), gameState(ACCEPTING_CONNECTIONS), isRunning(false) {}

// Destructor: Clean up resources
Server::~Server() {
    stop();
}

// Start the main thread
void Server::startCommandThread() {
    // std::thread handleConnections(&Server::start, this);
    threads.emplace_back(&Server::readCommands, this);
}

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
            // start this thread!
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
    

    if (state ==  STATES::WAITING_FOR_PLAYERS) {
        sendPhrase();
    }
    // Unlocks the threads awaiting for the game to start
    if (state ==  STATES::GAME_IN_PROGRESS) {
        sendStart();
        gameStateCV.notify_all();
    }
    if (state == STATES::ENDGAME) {
        sendEndgame();
    }

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

    while (isRunning && (gameState == ACCEPTING_CONNECTIONS)) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (!isRunning) break; // Stop accepting if server is stopping
            perror("ERROR on accept");
            continue;
        }

        initPlayerData(playerId);

        std::cout << "Player " << playerId << " connected." << std::endl;
        playerCount++;
        // Spawn a thread to handle the player
        threads.emplace_back(&Server::handlePlayer, this, clientSocket, playerId, clientAddr);


        // Sender socket
        // int senderSocket = socket(AF_INET, SOCK_STREAM, 0);

        // if (senderSocket < 0) {
        //     perror("ERROR creating socket");
        //     return;
        // }
        // // For now, the receiving port of the client is always the next from the sender
        // // addr.sin_port = htons(ntohs(addr.sin_port) + 1); 
        // clientAddr.sin_port = htons(12346); // TODO : no more gambiarra, create a system for the second port
        // socklen_t len = sizeof(clientAddr);
        // std::cout << "Attempting connection to " << clientAddr.sin_addr.s_addr << ":" << ntohs(clientAddr.sin_port) << std::endl;
        // if(connect(senderSocket, (sockaddr *)&clientAddr, len) < 0 ){
        //     perror("ERROR connecting to client");
        //     return;
        // }

        // playerConections[playerId] = {clientSocket, senderSocket};


        playerId++;
    }

    closeAllThreads();
    closeSocket();
}


void Server::waitForEndgame() {
    std::unique_lock<std::mutex> lck(completionTimesMutex);
    completionCV.wait(lck, [this] {return this->playerCount == this->finishedPlayers;});
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
    char buffer[1024];
    while (isRunning) {
        std::unique_lock<std::mutex> lck(gameStateMutex);
        // This stops the thread from blocking at the recv without busy waiting, because the client only sends one ready message
        if (gameState == STATES::WAITING_FOR_PLAYERS){
            gameStateCV.wait(lck, [this, player_id]{return playerReady[player_id];});
            lck.unlock();
        }

        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            std::cout << "Player " << player_id << " disconnected." << std::endl;
            playerCount--;
            playerConections.erase(playerConections.find(player_id));
            close(clientSocket);
            return;
        }

        ClientMessage message = ClientMessage::decode(std::string(buffer));

        switch (message.type) {
            case ClientMessage::ClientMessageType::READY :
                storePlayerReady(player_id);
                break;
            case ClientMessage::ClientMessageType::PROGRESS :
                updateRanking(message.data.name, message.data.score, message.data.timestamp);
                break;
            case ClientMessage::ClientMessageType::DONE :
                storePlayerDone(player_id, message.data.timestamp);
                break;
        default:
            break;
        }
    }
    close(clientSocket);
    
}


// TODO : esse mutex provavelmente fica mto pouco tempo desbloqueado, ent vai dar uns deadlock feio
// Precisa resolver isso com condition_variable (sleep/wakeup), faco isso amanha
void Server::sendPlayerData(int clientSocket, int player_id) {
    std::unique_lock<std::mutex> stateLock(gameStateMutex);
    while (isRunning) {

        switch (gameState) {
        case STATES::ACCEPTING_CONNECTIONS :
            // I think no messages are sent in this stage, in case i missed something, just put it here
        case STATES::WAITING_FOR_PLAYERS :
            // I think no messages are sent in this stage, in case i missed something, just put it here
            break;
        case STATES::GAME_IN_PROGRESS :
            sendRankings(clientSocket, player_id); 
            break;
        case STATES::ENDGAME :
            // Idk if the endgame messages are sent here or in other functions, vou dormir dps penso nessa bomba
            break;
        default:
            break;
        }
        // Signals other waiting thread that it can wakeup
        // Since its not blocking i hope this thread will often execute the next function
        messageSendingCV.notify_one();
        // Blocks the current thread and releases the mutex for other threads
        messageSendingCV.wait(stateLock, []{return true;}); 
    }
    close(clientSocket);
}

// Encapsulates the action of sending ranking messages
void Server::sendRankings(int clientSocket, int player_id) {
    std::unique_lock<std::mutex> rankLock(rankingsMutex);
    // ServerMessage message(playerCount, rankings, "", ServerMessage::ServerMessageType::RANKING);
    ServerMessage message;
    message.playerCount = playerCount;
    message.rankings = rankings;
    message.type = ServerMessage::ServerMessageType::RANKING;
    std::string str_message = message.encode();
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

// Handle a single player (to be substituted)
// server calculates which port the client will have to open based on its id
// and then sends it to them
void Server::handlePlayer(int clientSocket, int playerId, sockaddr_in addr) {

    int newPort = 12345 + playerId + 1;

    // send new port to client
    std::string newPortStr = std::to_string(newPort);
    if (send(clientSocket, newPortStr.c_str(), newPortStr.size(), 0) < 0) {
        std::cerr << "Error sending new port to client.\n";
        return;
    } 

    char buffer[1024];

    int senderSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (senderSocket < 0) {
        perror("ERROR creating socket");
        return;
    }
   
    addr.sin_port = htons(newPort); 
    socklen_t len = sizeof(addr);
    std::cout << "Attempting connection to " << addr.sin_addr.s_addr << ":" << ntohs(addr.sin_port) << std::endl;
    
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
            playerCount--;
            close(clientSocket);
            return;
        }

        // Process player's input
        std::string input(buffer);
        ClientMessage message = ClientMessage::decode(input);
        // std::cout << message.data.name << "[" << playerId << "]" << ": " << message.data.score << ", " << message.data.timestamp << std::endl;

        // Update rankings safely
        updateRanking(message.data.name, message.data.score, message.data.timestamp);

        // Send response to player
        // ServerMessage response;
        // std::string encodedresponse = response.encode();
        // if (send(senderSocket, encodedresponse.c_str(), encodedresponse.size(), 0) < 0) {
        //     std::cerr << "Error sending response to client.\n";
        //     break;
        // } 
            // std::cout << "Response sent to socket " << senderSocket << ": " << encodedresponse << std::endl;
    }

    close(clientSocket);
}

// Sends phrase message to all players
void Server::sendPhrase() {
    for (auto player_conn : playerConections) {
        std::string message =  ServerMessage(playerCount, "testphrase", ServerMessage::ServerMessageType::PHRASE).encode();
        if(send(player_conn.second.second, message.c_str(), message.size(), 0) < 0)
            std::cerr << "Error sending phrase to client.\n";
        else
            std::cout << "Phrase sent to " << player_conn.first << std::endl;
    }

}

// Sends start message to all players
void Server::sendStart() {
    for (auto player_conn : playerConections) {
        std::string message =  ServerMessage(playerCount, ServerMessage::ServerMessageType::START).encode();
        if(send(player_conn.second.second, message.c_str(), message.size(), 0) < 0)
            std::cerr << "Error sending start to client.\n";
        else
            std::cout << "Start sent to " << player_conn.first << std::endl;
    }

}

void Server::sendEndgame() {
    for (auto player_conn : playerConections) {
        ServerMessage message =  ServerMessage(playerCount, ServerMessage::ServerMessageType::END_GAME);
        std::unique_lock<std::mutex> lck(rankingsMutex);
        message.rankings = rankings;
        std::string str_m = message.encode();
        if(send(player_conn.second.second, str_m.c_str(), str_m.size(), 0) < 0)
            std::cerr << "Error sending start to client.\n";
        else
            std::cout << "Start sent to " << player_conn.first << std::endl;
    }
}

// this doesnt look efficient, should be changed, but if it works... 
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
    // std::cout << "Player " << playerName << "'s score updated to " << score << std::endl;
}

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
