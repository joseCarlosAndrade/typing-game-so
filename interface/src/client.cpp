#include "../include/client.hpp"

// #include <fcntl.h>

// Constructor
Client::Client(const std::string &ip, int portno) : serverIP(ip), senderPORT(portno), receivePORT(0) {}

// Destructor, frees resources
Client::~Client() {
    // Stops the sending of messages
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopSender = true;
    }   

    std::cout << "stopping sockets" << std::endl;
    // Closes and stops
    close(senderSocket);
    shutdown(senderSocket, SHUT_RDWR);
    close(receivingSocket);
    shutdown(receivingSocket, SHUT_RDWR);

    std::cout << "sockets off" << std::endl;

    // Wakes up sending thread so it can terminate
    queueCV.notify_all();

    // Joins threads

    if (senderThread.joinable()) {
        senderThread.join();
    }

    if (receiverThread.joinable()) {
        stopReceiver = true;
        receiverThread.join();
    }

    std::cout << "threads off" << std::endl;
}

// Same proccess as destructor, but called directly
void Client::clean() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopSender = true;
    }   

    std::cout << "[CLEAN] stopping sockets" << std::endl;

    close(senderSocket);
    shutdown(senderSocket, SHUT_RDWR);
    close(receivingSocket);
    shutdown(receivingSocket, SHUT_RDWR);

    std::cout << "[CLEAN] sockets off" << std::endl;

    queueCV.notify_all();
    if (senderThread.joinable()) {
        senderThread.join();
    }

    std::cout << "[CLEAN] sender stopped. waiting for receiver" << std::endl;

    if (receiverThread.joinable()) {
        stopReceiver = true;
        receiverThread.join();
    }

    std::cout << "[CLEAN] threads off" << std::endl;
}

// Estabilishes connection to the server
int Client::connectToServer() {

    // Creates the socket that sends data to the server

    senderSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (senderSocket < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(senderPORT);

    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        perror("ERROR invalid address");
        return -1;
    }

    // Connects to server
    if (connect(senderSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR connecting to server");
        return -1;
    }

    std::cout << "Connected to the server at " << serverIP << ":" << senderPORT << std::endl;

    // receives a message from the server with the correct port to open
    char buffer[1024];
    int bytesReceived = recv(senderSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        perror("ERROR receiving port from server");
        close(senderSocket);
        EXIT_FAILURE;
    }

    receivePORT = std::stoi(buffer);

    std::cout << "Received port " << receivePORT << " from server" << std::endl;


    /*
        The client keeps 2 sockets, 1 to receive data from the server, and one to send data to the server
    */

    // Starts the thread to send data to the server
    senderThread = std::thread(&Client::processQueue, this);

    // Starts the thread to receive data from the server
    receiverThread = std::thread(&Client::receiveUpdates, this, receivePORT);

    return 0;
}

// Adds data to sendQueue, thread safely
void Client::sendData(std::string data) {
    {
        // Gain control of mutex
        std::unique_lock<std::mutex> lock(queueMutex);
        sendQueue.push(data);
    }
    // Wake up consumer thread
    queueCV.notify_one();
}

// Creates message to be sent, and adds to the queue, thread safely
void Client::sendPosition(){
    ClientMessage message(interface.players[0].name, interface.keyboard->last_correct_index);
    std::string encodedMessage = message.encode();
    sendData(encodedMessage);
    // Adds delay sot not to clog the system when multiple interfaces and server are all running on the same computer 
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


// Receive messages from the server
void Client::receiveUpdates(int receivePort) {
    std::cout << "Receiving thread started" << std::endl;

    // First section of the function stes the socket

    if (receivePort == 0) {
        std::cout << "ERROR: receive port not set" << std::endl;
        return;
    }

    std::cout << "[RECEIVER] opening socket on port " << receivePort << std::endl;

    receivingSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (receivingSocket < 0){
        perror("ERROR opening receiving socket");
        return;
    }
    sockaddr_in recv_addr {};
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(receivePort);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    // Socket configuration

    // Reuse port, useful for testing
    int opt = 1;
    if (setsockopt(receivingSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(receivingSocket);
        exit(EXIT_FAILURE);
    }

    // Timeout, avoids locking threads indefintely
    struct timeval timeout;
    timeout.tv_sec = 1; // 1 seconds
    timeout.tv_usec = 0; // 0 microseconds
    if (setsockopt(receivingSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(receivingSocket);
        exit(EXIT_FAILURE);
    }

    if(bind(receivingSocket, (sockaddr *)&recv_addr, (socklen_t)sizeof(recv_addr)) < 0){
        perror("ERROR on binding");
        return;
    }

    if(listen(receivingSocket, 10) < 0){
        perror("ERROR on listening");
        return;
    }
    std::cout << "listening on port " << ntohs(recv_addr.sin_port) << std::endl;


    sockaddr_in new_recv_addr {};
    socklen_t socklen = sizeof(new_recv_addr);
    int recvSocket = accept(receivingSocket, (sockaddr *)&new_recv_addr, &socklen);
    if (recvSocket < 0){
        perror("ERROR on accepting");
        return;
    }
    std::cout << "Receiving connection estabilished" << std::endl;

    // At this point the socket is configured and ready

    // Receiving loop
    while(!stopReceiver){
        // Stores incoming message
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        
        std::cout << "[RECEIVER] waiting for message..\n";
        int bytesReceived = recv(recvSocket, buffer, sizeof(buffer) - 1, 0);
        std::cout << "[RECEIVER] ..message received\n";

        // Client stopping logic
        if (bytesReceived <= 0){
            continue;
        }
        //message decoding
        ServerMessage message = ServerMessage::decode(std::string(buffer));

        std::cout << "msg received\n";
        std::cout << "msg type: " << message.type << std::endl;
        
        // Handles each type of message accordingly
        switch (message.type) {
            // Sets the target phrase 
            case ServerMessage::ServerMessageType::PHRASE:

                if (interface.phrase_recieved == true) break;

                std::cout << "phrase received\n";
                std::cout <<"phrase: " << message.phrase << std::endl;
                
                
                interface.phrase_recieved = true;
                interface.setPhrase(message.phrase);
                break;
            // Starts the game
            case ServerMessage::ServerMessageType::START:
                std::cout << "game started\n";

                // creates a Player for each one received via message, but not the player related to this client, that one is create in interface.init
                for(auto data : message.rankings){
                    Player p;
                    SDL_Color c = {255, 255, 255, 255};
                    p.last_correct_index = 0;
                    p.actual_index = 0;
                    p.color = c;
                    p.name = data.first;
                    if ((p.name == interface.players[0].name) || (p.name == " "))
                        continue;
                    interface.players.push_back(p);
                }
                interface.game_started = true;
                break;
            // Updates ranking on the interface
            case ServerMessage::ServerMessageType::RANKING: {
                int i = 1;
                // receives a ranking list of the players
                for(auto data : message.rankings){

                    // let "my player" be the index 0 in interface players vector
                    if (data.first == interface.players[0].name)
                        continue;

                    // updates the score of each player
                    if (int(interface.players.size()) == i){ //TODO: ZERO ISSO AQUI DEVE TA ERRADO
                        // COLOQUEI ISSO PQ PLAYERS NAO TA INICIALIZADO   
                        Player p;
                        SDL_Color c = {255, 255, 255, 255};
                        p.last_correct_index = 0;
                        p.actual_index = 0;
                        p.color = c;
                        interface.players.push_back(p);
                    }
                    interface.players[i].last_correct_index = data.second.first;
                    // updates the name of the players in 1st, 2nd, ...
                    interface.players[i].name = data.first;
                    i++;
                }
                break;
                }
            // Ends game and stops client
            case ServerMessage::ServerMessageType::END_GAME:
                interface.ended_game = true;
                break;

            default:
                std::cout <<"[RECEIVER] invalid message type received" << std::endl;
                break;
        }
        
        // Critical region for updating score
        std::unique_lock<std::mutex> rankLock(rankingMutex);
        auto rankings = message.rankings;
        for(auto pair : rankings) std::cout << pair.first << ":" << pair.second.first << std::endl; 
        rankLock.unlock();
        std::cout << "Score updated" << std::endl;
    }
    // Closes resources
    std::cout << "Closing receiving socket" << std::endl;
    close(recvSocket);
    close(receivingSocket);
}

