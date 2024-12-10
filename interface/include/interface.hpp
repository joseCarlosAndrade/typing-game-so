#ifndef INTERFACE_H
#define INTERFACE_H

#include <iostream>
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include"keyboard.hpp"
#include "client.hpp"

#define FONT_SIZE 24
#define FONT_SPACING 5

typedef struct {
    SDL_Color color;
    std::string name;
    int last_correct_index;
    int actual_index;
} Player;

typedef std::vector<Player> vp;

class Interface {
    private:
        
        int width;
        int height;
        int FPS;

        // loop variables
        bool running, stopWindow;

        // Players that joined in this game
        vp players;

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Event event;

        std::string phrase;

        TTF_Font* font;
        int fontsize;
        
        Keyboard *keyboard;

    public:
        Interface();
        ~Interface();

        void init(std::string phrase, std::string name);
        void update(std::string name, Client& client);
        void render();
        bool PlayerFinished();
        void handleEvents();
        void checkExitGame();
        void clean();

        bool isRunning();
        bool stop();
        void setStop(bool stop);
        void setRunning(bool running);
        void incPlayerIndex();
        int getFPS();
        int getWidth();
        int getHeight();
        SDL_Renderer* getRenderer();
        SDL_Event* getEvent();
        SDL_Window* getWindow();

        // text handling
        int draw_letter_to_screen(int x, int y, char letter, SDL_Color color);
        int draw_player_position(int x, int y, SDL_Color color);
        int draw_rectangle_limits();
        void draw_black_rectangle(SDL_Rect outerRect);


        void renderPhrase(std::string phrase);
        void setPhrase(std::string phrase);
        void renderTypedText();
        void renderPlayerPosition(Player * player, int index);

        void renderRank(vp players);

};

#endif