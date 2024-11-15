#include "interface.hpp"

Interface::Interface(int width, int height, int FPS) 
    : width(width), height(height), FPS(FPS), running(false), stopWindow(false), window(nullptr), renderer(nullptr) {}

Interface::~Interface() {
    clean();
}

void Interface::init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    window = SDL_CreateWindow("Typing Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return;
    }

    running = true;
}

void Interface::update() {
    // Update game logic here
}

void Interface::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render game objects here

    SDL_RenderPresent(renderer);
}

// handle events iteratively
void Interface::handleEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        // Handle other events here
    }
}

void Interface::clean() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

bool Interface::isRunning() {
    return running;
}

bool Interface::stop() {
    return stopWindow;
}

void Interface::setStop(bool stop) {
    stopWindow = stop;
}

void Interface::setRunning(bool running) {
    this->running = running;
}

void Interface::setFPS(int FPS) {
    this->FPS = FPS;
}

int Interface::getFPS() {
    return FPS;
}

int Interface::getWidth() {
    return width;
}

int Interface::getHeight() {
    return height;
}

SDL_Renderer* Interface::getRenderer() {
    return renderer;
}

SDL_Event* Interface::getEvent() {
    return &event;
}

SDL_Window* Interface::getWindow() {
    return window;
}