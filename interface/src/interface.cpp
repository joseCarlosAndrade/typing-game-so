#include "interface.hpp"
#include <SDL2/SDL_ttf.h>

#define BORDER 5
#define PAD_RECTANGLE 40

SDL_Color white = {255, 255, 255, 255};
SDL_Color red = {255, 0, 0, 255};
SDL_Color faded = {255, 255, 255, 100};

Interface::Interface()
    : FPS(60), running(false), stopWindow(false), window(nullptr), renderer(nullptr), font(nullptr), fontsize(FONT_SIZE) {
    
    //    keyboard = Keyboard(100, 100, 10, 24);
    phrase_recieved = false;
    game_started = false;
    ended_game = false;
}

Interface::~Interface() {
    clean();
}

void Interface::init(std::string name) {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    width = DM.w * 0.8;
    height = DM.h * 0.8;

    keyboard = new Keyboard(3, width - (2*(FONT_SIZE + FONT_SPACING)), 3, height, FONT_SPACING, FONT_SIZE);

    std::cout << "Width: " << width << " Height: " << height << std::endl;

    window = SDL_CreateWindow("Typing Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF Initialization failed: " << TTF_GetError() << std::endl;
        return;
    }

    // load font
    font = TTF_OpenFont("assets/arial.ttf", fontsize);


    // add myplayer to the interface vector of players
    Player myplayer;
    SDL_Color c = {255, 255, 255, 255};
    myplayer.last_correct_index = 0;
    myplayer.actual_index = 0;
    myplayer.color = c;
    myplayer.name = name;
    players.push_back(myplayer);

    running = true;
}

void Interface::update() {
    // Update game logic here
    // will add players to the vector or remove
    players[0].last_correct_index = keyboard->last_correct_index;
    players[0].actual_index = keyboard->get_last_index();

}

void Interface::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render game objects here

    // write text to screen
    // draw_correct_letter_to_screen(100, 100, 'A');
    std::string phrase = keyboard->get_phrase();

    draw_rectangle_limits();

    if (phrase_recieved)
        renderPhrase(phrase);
    if (game_started){
        renderTypedText();
        renderRank(this->players);
        for (int i = 0; i < int(players.size()); i++){
            renderPlayerPosition(&players[i], players[i].last_correct_index);
        }
    }

    



    SDL_RenderPresent(renderer);
}

// handle events iteratively
void Interface::handleEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type){
            case SDL_QUIT:
                running = false;
                break;


            case SDL_KEYDOWN:
                SDL_Keycode key = event.key.keysym.sym;

                std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << " NUMBER: " << key << std::endl;
                if (key == SDLK_ESCAPE) {
                    running = false;

                } else if (!PlayerFinished() && game_started && phrase_recieved && !ended_game) {
                    if (key >= SDLK_a && key <= SDLK_z) {
                        char letter = key - SDLK_a + 'A'; // convert to uppercase
                        keyboard->insert_letter(letter);
                    } else if (key == SDLK_BACKSPACE) {
                        keyboard->delete_letter();
                    } else if (key == SDLK_SPACE) {
                        keyboard->insert_letter(' ');
                    }
                }
                break;

            
            // Handle other events here
        }
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

bool Interface::PlayerFinished(){
    if (keyboard->last_correct_index == (int(phrase.size()))){
        ended_game = true;
        return true;
    }
    return false;
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

void Interface::incPlayerIndex() {
    this->players[0].last_correct_index++;
    this->players[0].actual_index++;
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

int Interface::draw_letter_to_screen(int x, int y, char letter, SDL_Color color) {

    // conver char to char *
    char str[2];
    if ((letter == ' ') && (color.a == 255) && (color.b == 0) && (color.g == 0) && (color.r == 255))
        str[0] = '_';
    else 
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

int Interface::draw_player_position(int x, int y, SDL_Color color){
        // conver char to char *
    char str[2];
    str[0] = '_';
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

void Interface::draw_black_rectangle(SDL_Rect outerRect) {
    SDL_Rect innerRect;
    innerRect.x = outerRect.x + 5; // Adjust border thickness
    innerRect.y = outerRect.y + 5;
    innerRect.w = outerRect.w - 10; // Subtract border thickness on both sides
    innerRect.h = outerRect.h - 10;

    SDL_Surface *surface = SDL_CreateRGBSurface(0, innerRect.w, innerRect.h, 32, 0, 0, 0, 0);
    SDL_PixelFormat *pf = surface->format;
    int color = SDL_MapRGB(pf, 0, 0, 0);

    SDL_FillRect(surface, NULL, color);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);

    SDL_RenderCopy(renderer, texture, NULL, &innerRect);
    SDL_DestroyTexture(texture);
}

int Interface::draw_rectangle_limits() {
    vi box_delimiters = keyboard->get_box_delimeters();
    int maxWidth = box_delimiters[1];
    int maxHeight = box_delimiters[3];

    SDL_Rect outerRect;
    outerRect.x = 40;
    outerRect.y = 40;
    outerRect.w = maxWidth-PAD_RECTANGLE;
    outerRect.h = maxHeight / 2;

    SDL_Surface *surface = SDL_CreateRGBSurface(0, outerRect.w, outerRect.h, 32, 0, 0, 0, 0);
    SDL_PixelFormat *pf = surface->format;
    int color = SDL_MapRGB(pf, 255, 255, 255);

    SDL_FillRect(surface, NULL, color);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);

    SDL_RenderCopy(renderer, texture, NULL, &outerRect);
    SDL_DestroyTexture(texture);

    draw_black_rectangle(outerRect); // Pass the outer rectangle to calculate inner one

    return 0;
}

void Interface::setPhrase(std::string phrase){
    keyboard->setPhrase(phrase);
    this->phrase = keyboard->get_phrase();
}

void Interface::renderPhrase(std::string phrase){
    vi box_delimiters = keyboard->get_box_delimeters();
    int iniXpos = box_delimiters[0];
    int maxWidth = box_delimiters[1];
    int iniYpos = box_delimiters[2];
    int x = iniXpos;
    int y = iniYpos;

    for(int i = 0; i < int(phrase.size()); i++){

        draw_letter_to_screen(x*(fontsize + FONT_SPACING), y*(fontsize + FONT_SPACING), phrase[i], faded);
        x++;

        int final_width = (x + 1) * (fontsize + FONT_SPACING); 

        if (final_width > maxWidth){
            x = iniXpos;
            y++;
        }
    }
}

void Interface::renderTypedText(){
    auto positioned_text = keyboard->get_positioned_text();

    // iterate though the positioned text and draw it to the screen
    for (; !positioned_text.empty(); positioned_text.pop_front()) {
        PositionedLetter pl = positioned_text.front();

        
        if (pl.second.index <= keyboard->last_correct_index){

            
            if ((pl.first == phrase[pl.second.index])){
                draw_letter_to_screen(pl.second.x * (fontsize + FONT_SPACING), pl.second.y * (fontsize + FONT_SPACING), pl.first, white);
            } 
            else {
                draw_letter_to_screen(pl.second.x * (fontsize + FONT_SPACING), pl.second.y * (fontsize + FONT_SPACING), phrase[pl.second.index], red);
            }
        }
        
        
        else {
            if (pl.second.index < int(phrase.size())){
                draw_letter_to_screen(pl.second.x * (fontsize + FONT_SPACING), pl.second.y * (fontsize + FONT_SPACING), phrase[pl.second.index], red);
            }
            else {
                draw_letter_to_screen(pl.second.x * (fontsize + FONT_SPACING), pl.second.y * (fontsize + FONT_SPACING), ' ', red);
            }
        }

        
    }
}

void Interface::renderPlayerPosition(Player * player, int index){
    int aux = index;
    vi box_delimiters = keyboard->get_box_delimeters();
    int iniXpos = box_delimiters[0];
    int maxWidth = box_delimiters[1];
    int iniYpos = box_delimiters[2];
    

    int x = iniXpos;
    int y = iniYpos;

    for (int i = 0; i < aux; i++){
        int final_width = (x + 2) * (fontsize + FONT_SPACING);

        if (final_width > maxWidth){
            x = iniXpos;
            y++;
        }
        else {
            x++;
        }
    }

    draw_player_position(x*(fontsize + FONT_SPACING), y*(fontsize + FONT_SPACING), player->color);
}

void Interface::renderRank(vp players){
    int h = height - 2 * (fontsize + FONT_SPACING);
    for (int i = 0; i < int(players.size()); i++, h = h - 2 * (fontsize + FONT_SPACING)){
        int progress = 100*(float(players[i].last_correct_index)/float(phrase.size()));
        std::string progress_str = std::to_string(progress);
        std::string name = players[i].name;
        int letter_screen_pos = 1;
        for (int j = 0; j < int(name.size()); j++, letter_screen_pos++){
            draw_letter_to_screen(letter_screen_pos*(fontsize + FONT_SPACING/2), h, name[j], players[i].color);
        }
        draw_letter_to_screen(letter_screen_pos*(fontsize + FONT_SPACING/2), h, ' ', players[i].color);
        letter_screen_pos++;

        for (int j = 0; j < int(progress_str.size()); j++, letter_screen_pos++){
            draw_letter_to_screen(letter_screen_pos*(fontsize + FONT_SPACING/2), h, progress_str[j], players[i].color);
        }
        draw_letter_to_screen(letter_screen_pos*(fontsize + FONT_SPACING/2), h, '%', players[i].color);
        letter_screen_pos++;
    }
}
