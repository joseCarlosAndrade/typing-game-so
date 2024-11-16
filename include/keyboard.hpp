#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <iostream>
#include <bits/stdc++.h>

typedef struct {
    int x;
    int y;
} PositionIndex;

typedef std::pair<char, PositionIndex> PositionedLetter;

typedef std::deque<PositionedLetter> PositionedText;


// handler to deal with keyboard stacking letters logic
class Keyboard{
    private:
        PositionedText index_positioned_text;

        // stack to store the typed text
        // std::stack<char> typed_text;
        
        // phrase to be typed
        std::string phrase;

        // max screen values so it knows when to overflow text
        int maxWidth;
        int maxHeight;

        // space between letters. This is used to calculate the width of the text
        int font_spacing; 
        int font_size;

    public:
        Keyboard(int maxWidth, int maxHeight, int font_spacing, int font_size);
        ~Keyboard();

        // set the phrase to be typed
        void setPhrase(std::string phrase);

        // insert a letter to the typed text
        void insert_letter(char letter);

        // delete the last letter typed
        void delete_letter();

        // clear the typed text
        void clear_text();

        // get the typed text
        // std::string get_typed_text();

        // get the phrase
        std::string get_phrase();

        PositionedText get_positioned_text();
};

#endif
