#include "client.hpp"
#include "interface.hpp"
#include <iostream>

int main() {
    const std::string serverIP = "127.0.0.1"; // Server's IP address
    const int PORT = 12345;                   // Server's port number
    Client client(serverIP, PORT);

    if (client.connectToServer() != 0) {
        std::cerr << "Failed to connect to the server." << std::endl;
        return 1;
    }

    Interface interface = Interface();
    interface.init("testetestetestetestetestetestetestetestetestetestetestetestetestetestetestetestetestetest");

    while (interface.isRunning()) {
        interface.handleEvents();
        interface.update();
        interface.render();
    }

    interface.clean();

    return 0;
}
