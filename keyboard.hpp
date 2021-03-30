#pragma once
#include <string_view>

struct SDL_KeyboardEvent;

namespace keyboard {
std::string_view convert_to_input(SDL_KeyboardEvent *);
}
