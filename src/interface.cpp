#include "interface.hpp"
#include <SDL2/SDL_ttf.h>

Interface::Interface()
    : FPS(60), running(false), stopWindow(false), window(nullptr), renderer(nullptr) {

       
    }

Interface::~Interface() {
    clean();
}

void Interface::init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    width = DM.w * 0.8;
    height = DM.h * 0.8;

    std::cout << "Width: " << width << " Height: " << height << std::endl;

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

    if (TTF_Init() == -1) {
        std::cerr << "TTF Initialization failed: " << TTF_GetError() << std::endl;
        return;
    }

    // load font
    font = TTF_OpenFont("assets/arial.ttf", 24);

    running = true;
}

void Interface::update() {
    // Update game logic here
}

void Interface::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render game objects here
    
    // write text to screen
    SDL_Color color = {255, 255, 255, 255};
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Hello, World!", color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect;
    textRect.x = width/2;
    textRect.y = height/2;
    textRect.w = surface->w;
    textRect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);


    SDL_RenderPresent(renderer);
}

// handle events iteratively
void Interface::handleEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }

        if (event.type == SDL_KEYDOWN) {
            std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
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