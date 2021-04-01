#include "termhistory.hpp"

#include <iostream>

void TermHistory::start_row() {
    buff.clear();
}

void TermHistory::finish_row() {
    std::cout << "Row is history  =|";
    for(auto& c:buff){
        if(c.glyph.size() != 1) {
            std::cout << " ";
        }
        if(!std::isprint(*c.glyph.data())) {
            std::cout << " ";
        }
        std::cout << c.glyph;
    }
    std::cout << std::endl;
}

void TermHistory::add_cell_to_history(const gfx::TermCell& cell) {
    buff.push_back(cell);
}