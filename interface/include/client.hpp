#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "protocol.hpp"
#include "interface.hpp"

class Client {
private:
    std::string serverIP;
    int senderPORT;
    int senderSocket;

    // Receiving socket variables
    int receivePORT;
    int receivingSocket;

    std::queue<std::string> sendQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::thread senderThread;
    bool stopSender = false;

    std::mutex receiveMutex;
    std::thread receiverThread;
    bool stopReceiver = false;

    // Player rankings
    std::mutex rankingMutex;
    std::multimap<std::string, std::pair<int, int>> rankings;

    void processQueue() {
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this]() { return !sendQueue.empty() || stopSender; });

            if (stopSender && sendQueue.empty()) break;

            auto data = sendQueue.front();
            sendQueue.pop();
            lock.unlock();

            if (!data.empty() && send(senderSocket, data.c_str(), data.size(), 0) < 0) {
                perror("ERROR sending data to server");
            }
        }
    }

public:
    Client(const std::string &ip, int portno);
    ~Client();
    int connectToServer();
    void sendData(std::string data);
    void sendPosition();
    void receiveUpdates();

    Interface interface;
};

#endif
