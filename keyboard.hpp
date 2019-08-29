#pragma once
#include <string_view>

class SDL_KeyboardEvent;

namespace keyboard {
std::string_view convert_to_input(SDL_KeyboardEvent *);
}
