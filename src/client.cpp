#include "client.hpp"

Client::Client(const std::string &ip, int portno) : serverIP(ip), PORT(portno) {}

Client::~Client() {
    close(clientSocket);
}

int Client::connectToServer() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("ERROR opening socket");
        return -1;
    }

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

void Client::sendInput() {
    std::thread sender([this]() {
        while (true) {
            std::string input;
            std::cout << "Type a word: ";
            std::getline(std::cin, input);

            if (input.empty()) continue;

            if (send(clientSocket, input.c_str(), input.size(), 0) < 0) {
                perror("ERROR sending data to server");
                break;
            }
        }
    });

    sender.detach(); // Detach to allow other functions to run concurrently
}

void Client::sendData(std::string data) {
    std::thread sender([this, data]() {
        while (true) {
            if (data.empty()) continue;

            if (send(clientSocket, data.c_str(), data.size(), 0) < 0) {
                perror("ERROR sending data to server");
                break;
            }
        }
    });

    sender.detach();
}

void Client::receiveUpdates() {
    std::thread receiver([this]() {
        char buffer[1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived <= 0) {
                perror("ERROR receiving data from server");
                break;
            }

            std::cout << "Server: " << buffer << std::endl;
        }
    });

    receiver.detach(); // Detach to allow other functions to run concurrently
}
