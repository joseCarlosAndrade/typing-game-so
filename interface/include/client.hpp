#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Client {
private:
    std::string serverIP;
    int PORT;
    int clientSocket;

public:
    Client(const std::string &ip, int portno);
    ~Client();
    int connectToServer();
    void sendInput();
    void sendData(std::string data);
    void receiveUpdates();
};

#endif
