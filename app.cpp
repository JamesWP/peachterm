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

void VTerm::text_input(const char *input, size_t len) {
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
        default: {
        }
        }
      } break;
      case SDL_TEXTINPUT: {
        char *input = e.text.text;
        size_t len = strnlen_s(input, sizeof(decltype(e.text.text)));
        term.text_input(input, len);
      } break;
      default: {
      }
      }

      term.window.redraw();
    }
  }
}
