#ifndef INTERFACE_H
#define INTERFACE_H

#include <iostream>
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include"keyboard.hpp"

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



        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Event event;

        std::string phrase;

        TTF_Font* font;
        int fontsize;
        
        

    public:

        bool phrase_recieved;
        bool game_started;
        bool ended_game;

        // Players that joined in this game
        vp players;
        Keyboard *keyboard;

        Interface();
        ~Interface();

        void init(std::string name);
        void update();
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