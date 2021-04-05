#pragma once
#include <string_view>

struct SDL_KeyboardEvent;

namespace keyboard {

enum class Mode {Application, Normal};

std::string_view convert_to_input(SDL_KeyboardEvent *, Mode);
}
