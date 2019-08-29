#include "keyboard.hpp"

#include <SDL.h>
#include <cstdio>
#include <unordered_map>


namespace keyboard {

static const std::unordered_map<int, std::pair<int /*num*/, char /*letter*/>>
    lore{
        {SDLK_F1, {1, 'P'}},   {SDLK_LEFT, {1, 'D'}},
        {SDLK_F2, {1, 'Q'}},   {SDLK_RIGHT, {1, 'C'}},
        {SDLK_F3, {1, 'R'}},   {SDLK_UP, {1, 'A'}},
        {SDLK_F4, {1, 'S'}},   {SDLK_DOWN, {1, 'B'}},
        {SDLK_F5, {15, '~'}},  {SDLK_HOME, {1, 'H'}},
        {SDLK_F6, {17, '~'}},  {SDLK_END, {1, 'F'}},
        {SDLK_F7, {18, '~'}},  {SDLK_INSERT, {2, '~'}},
        {SDLK_F8, {19, '~'}},  {SDLK_DELETE, {3, '~'}},
        {SDLK_F9, {20, '~'}},  {SDLK_PAGEUP, {5, '~'}},
        {SDLK_F10, {21, '~'}}, {SDLK_PAGEDOWN, {6, '~'}},
        {SDLK_F11, {23, '~'}}, {SDLK_F12, {24, '~'}},
    };

static const std::unordered_map<int, char> lore2{
    {SDLK_a, 'a'},
    {SDLK_b, 'b'},
    {SDLK_c, 'c'},
    {SDLK_d, 'd'},
    {SDLK_e, 'e'},
    {SDLK_f, 'f'},
    {SDLK_g, 'g'},
    {SDLK_h, 'h'},
    {SDLK_i, 'i'},
    {SDLK_j, 'j'},
    {SDLK_k, 'k'},
    {SDLK_l, 'l'},
    {SDLK_m, 'm'},
    {SDLK_n, 'n'},
    {SDLK_o, 'o'},
    {SDLK_p, 'p'},
    {SDLK_q, 'q'},
    {SDLK_r, 'r'},
    {SDLK_s, 's'},
    {SDLK_t, 't'},
    {SDLK_u, 'u'},
    {SDLK_v, 'v'},
    {SDLK_w, 'w'},
    {SDLK_x, 'x'},
    {SDLK_y, 'y'},
    {SDLK_z, 'z'},
    {SDLK_ESCAPE, '\33'},
    {SDLK_0, '0'},
    {SDLK_1, '1'},
    {SDLK_2, '2'},
    {SDLK_3, '3'},
    {SDLK_4, '4'},
    {SDLK_5, '5'},
    {SDLK_6, '6'},
    {SDLK_7, '7'},
    {SDLK_8, '8'},
    {SDLK_9, '9'},
    {SDLK_PERIOD, '.'},
    {SDLK_COMMA, '-'},
    {SDLK_RETURN, '\r'},
    {SDLK_BACKSPACE, '\177'},
    {SDLK_TAB, '\t'},
    {SDLK_SPACE, ' '},
};

std::string_view convert_to_input(SDL_KeyboardEvent *e) {
  static char buffer[32];

  SDL_Keymod modifiers = SDL_GetModState();

  bool shift = modifiers & SDLK_LSHIFT || modifiers & SDLK_RSHIFT;
  bool alt = modifiers & SDLK_LALT || modifiers & SDLK_RALT;
  bool ctrl = modifiers & SDLK_LCTRL || modifiers & SDLK_RCTRL;

  int sym = e->keysym.sym;

  switch (sym) {
  case SDLK_RETURN:
    return "\n";
  case SDLK_BACKSPACE:
    return "\b";
  }

  if (auto i = lore.find(sym); i != lore.end()) {
    const auto &d = i->second;

    size_t delta = 1 + shift * 1 + alt * 2 + ctrl * 4;

    char bracket = '[';

    if (d.second >= 'P' && d.second <= 'S')
      bracket = 'O';
    if (d.second >= 'A' && d.second <= 'D' && delta == 1)
      bracket = 'O';

    if (delta != 1)
      return {buffer, (size_t)std::sprintf(buffer, "\33%c%d;%d%c", bracket,
                                           d.first, (int)delta, d.second)};
    else if (d.first == 1)
      return {buffer,
              (size_t)std::sprintf(buffer, "\33%c%c", bracket, d.second)};
    else
      return {buffer, (size_t)std::sprintf(buffer, "\33%c%d%c", bracket,
                                           d.first, d.second)};

  } else if (auto i = lore2.find(sym); i != lore2.end()) {
    char &cval = buffer[0] = i->second;

    bool digit = cval >= '0' && cval <= '9';
    bool alpha = cval >= 'a' && cval <= 'z';

    // Turn uppercase.
    if (shift && alpha) {
      cval &= ~0x20;
    }

    if (ctrl && digit){
      cval = "01\0\33\34\35\36\37\1779"[cval - '0'];
    }

    if (ctrl && i->second == '\177'){
      cval = '\b';
    } else if (ctrl && !digit) {
      // Turn into a control character.
      cval &= 0x1F;
    }

    // CTRL a..z becomes 01..1A
    // CTRL 0..9 becomes 10..19, should become xx,xx,00,1B-1F,7F,xx

    // Add ALT.
    if (alt) {
      cval |= 0x80; 
    }

    if ((!alpha && !digit) || ctrl || alt) {
      if (shift && cval == '\t') {
        return "\33[Z";
      }
      return {buffer, 1u};
    }
  }

  return {nullptr, 0u};
}
} // namespace keyboard
