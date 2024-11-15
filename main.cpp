#include"interface.hpp"
#include<iostream>


int main() {
    Interface interface(800, 600, 60);
    interface.init();

    while (interface.isRunning()) {
        interface.handleEvents();
        interface.update();
        interface.render();
    }

    interface.clean();

    return 0;
}
