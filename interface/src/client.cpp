#include "../include/client.hpp"

// #include <fcntl.h>


Client::Client(const std::string &ip, int portno) : serverIP(ip), senderPORT(portno), receivePORT(0) {}

Client::~Client() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopSender = true;
    }   

    std::cout << "stopping sockets" << std::endl;

    close(senderSocket);
    shutdown(senderSocket, SHUT_RDWR);
    close(receivingSocket);
    shutdown(receivingSocket, SHUT_RDWR);

    std::cout << "sockets off" << std::endl;

    queueCV.notify_all();
    if (senderThread.joinable()) {
        senderThread.join();
    }

    if (receiverThread.joinable()) {
        stopReceiver = true;
        receiverThread.join();
    }

    std::cout << "threads off" << std::endl;
}

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

int Client::connectToServer() {
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

    senderThread = std::thread(&Client::processQueue, this);

    receiverThread = std::thread(&Client::receiveUpdates, this, receivePORT);

    return 0;
}

void Client::sendData(std::string data) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        sendQueue.push(data);
    }
    queueCV.notify_one();
}

void Client::sendPosition(){
    ClientMessage message(interface.players[0].name, interface.keyboard->last_correct_index);
    std::string encodedMessage = message.encode();
    sendData(encodedMessage);
}


// TODO : close sockets correctly, create a real system for the second port
void Client::receiveUpdates(int receivePort) {
    std::cout << "Receiving thread started" << std::endl;

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

    int opt = 1;
    if (setsockopt(receivingSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(receivingSocket);
        exit(EXIT_FAILURE);
    }

    struct timeval timeout;
    timeout.tv_sec = 1; // 1 seconds
    timeout.tv_usec = 0; // 0 microseconds
    if (setsockopt(receivingSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(receivingSocket);
        exit(EXIT_FAILURE);
    }

    // int flags = fcntl(receivingSocket, F_GETFL, 0);
    // fcntl(receivingSocket, F_SETFL, flags & ~O_NONBLOCK);

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

    while(!stopReceiver){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        
        std::cout << "[RECEIVER] waiting for message..\n";
        int bytesReceived = recv(recvSocket, buffer, sizeof(buffer) - 1, 0);
        std::cout << "[RECEIVER] ..message received\n";

        if (bytesReceived <= 0){
            // std::cout << "Lost connection to server" << std::endl;
            // break;
            continue;
        }

        ServerMessage message = ServerMessage::decode(std::string(buffer));

        std::cout << "msg received\n";
        std::cout << "msg type: " << message.type << std::endl;
        
        //std::cout << "a\n"; 
        switch (message.type) {
            case ServerMessage::ServerMessageType::PHRASE:

                if (interface.phrase_recieved == true) break;

                std::cout << "phrase received\n";
                std::cout <<"phrase: " << message.phrase << std::endl;
                
                
                interface.phrase_recieved = true;
                interface.setPhrase(message.phrase);
                break;

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

            case ServerMessage::ServerMessageType::RANKING: {
                int i = 1;
                std::cout << "Rankings:" << std::endl;
                for(auto data : message.rankings){
                    std::cout << data.first << " " << data.second.first << " " << data.second.second << std::endl;
                }
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

    std::cout << "Closing receiving socket" << std::endl;
    close(recvSocket);
    close(receivingSocket);
}

