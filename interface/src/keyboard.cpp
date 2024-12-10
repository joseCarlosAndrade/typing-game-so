#include "keyboard.hpp"

Keyboard::Keyboard(int iniXpos, int maxWidth, int iniYpos, int maxHeight, int font_spacing , int font_size) 
    :iniXpos(iniXpos), maxWidth(maxWidth), iniYpos(iniYpos), maxHeight(maxHeight), font_spacing(font_spacing), font_size(font_size) {
    phrase = "";
    last_index = 0;
    last_correct_index = 0;
}

Keyboard::~Keyboard() {
    phrase = "";
}


void Keyboard::setPhrase(std::string phrase) {
    this->phrase = phrase;
    std::transform(this->phrase.begin(), this->phrase.end(), this->phrase.begin(), ::toupper);
}

std::string Keyboard::get_phrase() {
    return phrase;
}

void Keyboard::insert_letter(char letter) {
    // typed_text.push(letter);
    PositionedLetter pl;
    pl.first = letter;
    pl.second.index = last_index;
    if ((letter == phrase[last_correct_index]) && (last_correct_index >= last_index)){
        last_correct_index++;
    }
    last_index++;



    // if empty, just isnert at 0,0
    if (index_positioned_text.empty()) {
        pl.second.x = iniXpos;
        pl.second.y = iniYpos;
        index_positioned_text.push_back(pl);

    } else {
        // if not, check the last letter to see if there is space to insert the new letter
        // if theres no space, insert on the next line
        PositionedLetter last_letter = index_positioned_text.back();

        // simulating the final width if the letter is inserted
        int final_width = (last_letter.second.x + 2) * (font_spacing + font_size); 

        if (final_width > maxWidth) {
            // insert on the next line
            pl.second.x = iniYpos;
            pl.second.y = last_letter.second.y + 1;
            index_positioned_text.push_back(pl);
        } else {
            // insert on the same line
            pl.second.x = last_letter.second.x + 1;
            pl.second.y = last_letter.second.y;
            index_positioned_text.push_back(pl);
        }
    }

}

void Keyboard::delete_letter() {

    if (!index_positioned_text.empty()) {
        index_positioned_text.pop_back();
        if (last_index == last_correct_index)
            last_correct_index--;
        last_index--;
    }
}

PositionedText Keyboard::get_positioned_text() {
    return index_positioned_text;
}

int Keyboard::get_last_index() {
    return last_index;
}

void Keyboard::clear_text() {
    // while (!typed_text.empty()) {
    //     typed_text.pop();
    // }
}

vi Keyboard::get_box_delimeters(){
    vi vet;
    vet.push_back(iniXpos);
    vet.push_back(maxWidth);
    vet.push_back(iniYpos);
    vet.push_back(maxHeight);
    return vet;
}

// std::string Keyboard::get_typed_text() {
//     std::string text;
//     // text.reserve(typed_text.size());
//     // std::stack<char> temp = typed_text;

//     // int index = typed_text.size() -1;
//     // while (!temp.empty()) {
//     //     text[index--] = temp.top();
//     //     temp.pop();
//     // }
//     return 
    

//     return text;
// }