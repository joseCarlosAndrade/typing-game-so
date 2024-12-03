#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "protocol.hpp"

class Client {
private:
    std::string serverIP;
    int PORT;
    int clientSocket;

    std::queue<std::string> sendQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::thread senderThread;
    bool stopSender = false;

    std::mutex receiveMutex;
    std::thread receiverThread;

    void processQueue() {
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this]() { return !sendQueue.empty() || stopSender; });

            if (stopSender && sendQueue.empty()) break;

            auto data = sendQueue.front();
            sendQueue.pop();
            lock.unlock();

            if (!data.empty() && send(clientSocket, data.c_str(), data.size(), 0) < 0) {
                perror("ERROR sending data to server");
            }
        }
    }

public:
    Client(const std::string &ip, int portno);
    ~Client();
    int connectToServer();
    void sendData(std::string data);
    void receiveUpdates();
};

#endif
