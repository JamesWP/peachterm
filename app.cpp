#include "app.hpp"
#include "graphics.hpp"
#include "io.hpp"

#include <SDL.h>
#include <chrono>
#include <iostream>
#include <string.h>

namespace app {
VTerm::VTerm() {
  window.resize_window(rows, cols);
  cell.fg_col = 0xFFFFFFFF;
  cell.bg_col = 0x000000FF;
}

void VTerm::putglyph(const char *input, size_t len) {
  cell.glyph.assign(input, len);
  window.set_cell(row, col, cell);

  col++;

  if (col >= cols) {
    col = 0;
    row++;
  }

  if (row >= rows) {
    row = rows - 1;
  }
}

void VTerm::newline() {
  row++;
  col = 0;
  if (row >= rows) {
    row = rows - 1;
  }
}

void VTerm::backspace() {
  col--;
  cell.glyph.assign(" ", 1);
  window.set_cell(row, col, cell);
  if (col < 0) {
    col = 0;
  }
}

void VTerm::toggle_bold() { cell.bold = !cell.bold; }

void VTerm::toggle_italic() { cell.italic = !cell.italic; }

void VTerm::set_fg_red(int red) {
  cell.fg_col &= 0x00FFFFFF;
  cell.fg_col |= (red & 0xFF) << 24;
}

void VTerm::set_fg_green(int green) {
  cell.fg_col &= 0xFF00FFFF;
  cell.fg_col |= (green & 0xFF) << 16;
}

void VTerm::set_fg_blue(int blue) {
  cell.fg_col &= 0xFFFF00FF;
  cell.fg_col |= (blue & 0xFF) << 8;
}

int VTerm::fg_red() const { return ((cell.fg_col & 0xFF000000) >> 24) & 0xFF; }

int VTerm::fg_green() const {
  return ((cell.fg_col & 0x00FF0000) >> 16) & 0xFF;
}

int VTerm::fg_blue() const { return ((cell.fg_col & 0x0000FF00) >> 8) & 0xFF; }

} // namespace app

size_t strnlen_s(const char* s, size_t len)
{
  for (size_t size = 0; size < len; size++)
    if (s[0] == '\0')
      return size;
    else
      s++;
  return 0;
}

void app::run()
{
  std::cout << "App run\n";

  app::VTerm term;

  io::PseudoTerminal pt([]() { std::cout << "PT read available\n"; });

  pt.start();

  SDL_Event e;

  while (true) {
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        return;
      case SDL_KEYDOWN: {
        switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          return;
        case SDLK_RETURN:
          term.newline();
          break;
        case SDLK_BACKSPACE:
          term.backspace();
          break;
        case SDLK_b:
          if (e.key.keysym.mod & KMOD_CTRL) {
            term.toggle_bold();
          }
          break;
        case SDLK_i:
          if (e.key.keysym.mod & KMOD_CTRL) {
            term.toggle_italic();
          }
          break;
        case SDLK_1:
          if (e.key.keysym.mod & KMOD_CTRL) {
            bool up = !(e.key.keysym.mod & KMOD_SHIFT);
            term.set_fg_red(term.fg_red() + (up ? 20 : -20));
          }
          break;
        case SDLK_2:
          if (e.key.keysym.mod & KMOD_CTRL) {
            bool up = !(e.key.keysym.mod & KMOD_SHIFT);
            term.set_fg_green(term.fg_green() + (up ? 20 : -20));
          }
          break;
        case SDLK_3:
          if (e.key.keysym.mod & KMOD_CTRL) {
            bool up = !(e.key.keysym.mod & KMOD_SHIFT);
            term.set_fg_blue(term.fg_blue() + (up ? 20 : -20));
          }
          break;
        default: {
        }
        }
      } break;
      case SDL_TEXTINPUT: {
        char *input = e.text.text;
        size_t len = strnlen_s(input, sizeof(decltype(e.text.text)));
        term.putglyph(input, len);
      } break;
      default: {
      }
      }

      term.window.redraw();
    }
  }
}
