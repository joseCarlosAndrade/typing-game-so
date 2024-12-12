#include "server.hpp"

// Main funtction for the server, starts its functionalities
int main() {
    Server server(12345);
    server.startCommandThread();
    server.start();

    return 0;
}