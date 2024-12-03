#include "client.hpp"

Client::Client(const std::string &ip, int portno) : serverIP(ip), PORT(portno) {}

Client::~Client() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopSender = true;
    }
    queueCV.notify_all();
    if (senderThread.joinable()) {
        senderThread.join();
    }

    close(clientSocket);
}

int Client::connectToServer() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    senderThread = std::thread(&Client::processQueue, this);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        perror("ERROR invalid address");
        return -1;
    }

    if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR connecting to server");
        return -1;
    }

    std::cout << "Connected to the server at " << serverIP << ":" << PORT << std::endl;
    return 0;
}

void Client::sendData(std::string data) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        sendQueue.push(data);
    }
    queueCV.notify_one();
}

void Client::receiveUpdates() {
    
}
