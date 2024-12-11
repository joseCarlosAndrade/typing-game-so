#include "client.hpp"
#include "interface.hpp"
#include <iostream>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cout << "Please write your name when running the program:" << std::endl;
        std::cout << "make run NAME=your_name" << std::endl;
        exit(1);
    }
    std::cout << "Welcome, " << argv[1] << std::endl;

    const std::string serverIP = "127.0.0.1"; // Server's IP address
    const int PORT = 12345;                   // Server's port number
    Client client(serverIP, PORT);

    if (client.connectToServer() != 0) {
        std::cerr << "Failed to connect to the server." << std::endl;
        return 1;
    }

    client.interface.init(argv[1]);

    while (client.interface.isRunning()) {
        client.interface.handleEvents();
        client.interface.update();
        client.sendPosition();
        client.interface.render();
    }

    client.interface.clean();

    return 0;
}
