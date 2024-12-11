#include "client.hpp"

Client::Client(const std::string &ip, int portno) : serverIP(ip), senderPORT(portno), receivePORT(portno+1) {}

Client::~Client() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopSender = true;
    }
    queueCV.notify_all();
    if (senderThread.joinable()) {
        senderThread.join();
    }

    close(senderSocket);
}

int Client::connectToServer() {
    senderSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (senderSocket < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    senderThread = std::thread(&Client::processQueue, this);

    receiverThread = std::thread(&Client::receiveUpdates, this);

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
void Client::receiveUpdates() {
    receivingSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (receivingSocket < 0){
        perror("ERROR opening receiving socket");
        return;
    }
    sockaddr_in recv_addr {};
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(receivePORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(receivingSocket, (sockaddr *)&recv_addr, (socklen_t)sizeof(recv_addr)) < 0){
        perror("ERROR on binding");
        return;
    }

    if(listen(receivingSocket, 5) < 0){
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
        int bytesReceived = recv(recvSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0){
            std::cout << "Lost connection to server" << std::endl;
        }

        ServerMessage message = ServerMessage::decode(std::string(buffer));
        
        std::cout << "a\n";
        switch (message.type) {
            case ServerMessage::ServerMessageType::PHRASE:
                interface.phrase_recieved = true;
                interface.setPhrase(message.phrase);
                break;

            case ServerMessage::ServerMessageType::START:

                // creates a Player for each one received via message, but not the player related to this client, that one is create in interface.init
                for(auto data : message.rankings){
                    Player p;
                    SDL_Color c = {255, 255, 255, 255};
                    p.last_correct_index = 0;
                    p.actual_index = 0;
                    p.color = c;
                    p.name = data.first;
                    if (p.name == interface.players[0].name)
                        continue;
                    interface.players.push_back(p);
                }
                interface.game_started = true;
                break;

            case ServerMessage::ServerMessageType::RANKING: {
                int i = 1;
                // receives a ranking list of the players
                for(auto data : message.rankings){

                    // let "my player" be the index 0 in interface players vector
                    if (data.first == interface.players[0].name)
                        continue;

                    // updates the score of each player
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
                break;
        }
        
        // Critical region for updating score
        std::unique_lock<std::mutex> rankLock(rankingMutex);
        auto rankings = message.rankings;
        for(auto pair : rankings) std::cout << pair.first << ":" << pair.second.first << std::endl; 
        rankLock.unlock();
        std::cout << "Score updated" << std::endl;
    }
    close(recvSocket);

}
