#ifndef INTERFACE_H
#define INTERFACE_H

#include <iostream>
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include"keyboard.hpp"

#define FONT_SIZE 24
#define FONT_SPACING 5

class Interface {
    private:
        
        int width;
        int height;
        int FPS;

        // loop variables
        bool running, stopWindow;

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Event event;

        TTF_Font* font;
        int fontsize;
        
        Keyboard *keyboard;

    public:
        Interface();
        ~Interface();

        void init();
        void update();
        void render();
        void handleEvents();
        void clean();

        bool isRunning();
        bool stop();
        void setStop(bool stop);
        void setRunning(bool running);
        int getFPS();
        int getWidth();
        int getHeight();
        SDL_Renderer* getRenderer();
        SDL_Event* getEvent();
        SDL_Window* getWindow();

        // text handling
        int draw_letter_to_screen(int x, int y, char letter);

};

#endif