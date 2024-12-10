#include "server.hpp"

int main() {
    Server server(12345);
    server.startCommandThread();
    server.start();

    return 0;
}