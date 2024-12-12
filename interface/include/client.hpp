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
    // Connection info
    std::string serverIP;
    int senderPORT;
    int senderSocket;

    // Receiving socket variables
    int receivePORT;
    int receivingSocket;

    std::queue<std::string> sendQueue; // Stores data to be sent
    std::mutex queueMutex; // sendQueue mutex semaphore
    std::condition_variable queueCV; // Condition variable, avois busy waiting and deadlocks
    std::thread senderThread; // Thread that runs proccessQueue and sends data to server
    bool stopSender = false; // Flag to halt sending

    std::mutex receiveMutex; // Mutex for receiving data
    std::thread receiverThread; // Thread for running receiveUpdates, and receive data from server
    bool stopReceiver = false; // Flag to halt receiving

    // Player rankings
    std::mutex rankingMutex; // Controls access to rankings
    std::multimap<std::string, std::pair<int, int>> rankings; //{name, {score, timestamps}}

    // Data to be sent is stored in a queue, in a classic producer-consumer fashion
    // This function, ran as a thread is responsible for managing the queue
    void processQueue() {
        // Access the queue thread safely
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            // Sleeps if the queue is empty
            queueCV.wait(lock, [this]() { return !sendQueue.empty() || stopSender; });

            // Stop conditions
            if (stopSender && sendQueue.empty()) break;

            // Access data and unlock the mutex
            auto data = sendQueue.front();
            sendQueue.pop();
            lock.unlock();
            
            // Sends data to the server
            if (!data.empty() && send(senderSocket, data.c_str(), data.size(), 0) < 0) {
                perror("ERROR sending data to server");
            }
        }
    }

public:
    Client(const std::string &ip, int portno);
    ~Client();
    int connectToServer(); // Estabilishes connections to the server
    void sendData(std::string data); // Adds data to sendQueue
    void sendPosition(); 
    void receiveUpdates(int receivePort); // Ran as thread, receives data from server

    void clean(); // Frees resources

    Interface interface; // Graphic interface
};

#endif
