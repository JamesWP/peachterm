#pragma once

#include <type_traits>
#include <vector>

#include "termcell.hpp"

class TermHistory {
    std::vector<gfx::TermCell> buff;

public:
    template<typename It>
    void add_row_to_history(It lineBegin, It lineEnd);

private:
    void start_row();
    void add_cell_to_history(const gfx::TermCell&);
    void finish_row();
};

template<typename It>
inline void TermHistory::add_row_to_history(It lineBegin, It lineEnd) {
    //static_assert(std::is_same_v<It::value_type, gfx::TermCell>);

    start_row();
    for(auto& c=lineBegin; c != lineEnd; ++c) {
        add_cell_to_history(c->value());
    }
    finish_row();
}