#include "interface.hpp"
#include <SDL2/SDL_ttf.h>

Interface::Interface()
    : FPS(60), running(false), stopWindow(false), window(nullptr), renderer(nullptr), font(nullptr), fontsize(FONT_SIZE) {

    //    keyboard = Keyboard(100, 100, 10, 24);
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

    keyboard = new Keyboard(width, height, FONT_SPACING, FONT_SIZE);

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
    font = TTF_OpenFont("assets/arial.ttf", fontsize);

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
    // draw_letter_to_screen(100, 100, 'A');

    auto positioned_text = keyboard->get_positioned_text();

    // iterate though the positioned text and draw it to the screen
    for (; !positioned_text.empty(); positioned_text.pop_front()) {
        PositionedLetter pl = positioned_text.front();
        draw_letter_to_screen(pl.second.x * (fontsize + FONT_SPACING), pl.second.y * (fontsize + FONT_SPACING), pl.first);
    }

    SDL_RenderPresent(renderer);
}

// handle events iteratively
void Interface::handleEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }

        if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;

            std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << " NUMBER: " << key << std::endl;
            if (key == SDLK_ESCAPE) {
                running = false;
            } else if (key >= SDLK_a && key <= SDLK_z) {
                char letter = key - SDLK_a + 'A'; // convert to uppercase
                keyboard->insert_letter(letter);
            } else if (key == SDLK_BACKSPACE) {
                keyboard->delete_letter();
            } else if (key == SDLK_SPACE) {
                keyboard->insert_letter(' ');
            }
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

int Interface::getFPS() {
    return FPS;
}

int Interface::getWidth() {
    return width;
}

int Interface::getHeight() {
    return height;
}

SDL_Renderer *Interface::getRenderer() {
    return renderer;
}

SDL_Event *Interface::getEvent() {
    return &event;
}

SDL_Window *Interface::getWindow() {
    return window;
}

int Interface::draw_letter_to_screen(int x, int y, char letter) {
    // we dont need to init / clear / render anything, since this function will be called inside render()

    SDL_Color color = {255, 255, 255, 255};

    // conver char to char *
    char str[2];
    str[0] = letter;
    str[1] = '\0';

    SDL_Surface *surface = TTF_RenderText_Solid(font, str, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect;
    textRect.x = x;
    textRect.y = y;
    textRect.w = surface->w;
    textRect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);

    return 0;
}