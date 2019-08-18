#include "app.hpp"
#include "graphics.hpp"
#include "io.hpp"
#include "parser.hpp"

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
  cell.glyph.assign("\xF0\x9F\x98\x82", 4u);
  window.set_cell(row, col, cell);
  return;

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

class App : public parser::VTParser, public app::VTerm
{
    public:
      void on_glyph(const char *data, size_t length) {
        std::cout << "on_glyph\n";
        putglyph(data, length);
      }
};

void app::run()
{
  std::cout << "App run\n";

  App term;

  const uint32_t data_available_event = SDL_RegisterEvents(1);

  SDL_Event data_available;
  SDL_memset(&data_available, 0, sizeof(data_available));
  data_available.type = data_available_event;

  io::PseudoTerminal pt([&data_available](io::PseudoTerminal *, const char *data,
                           size_t length) {
    data_available.user.data1 = static_cast<void*>(const_cast<char*>(data));
    data_available.user.data2 = reinterpret_cast<void*>(length);
    SDL_PushEvent(&data_available);
  });

  pt.start();

  SDL_Event e;

  while (true) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == data_available_event) {
        std::cout << "SDL Event\n";
        term.parse_input(static_cast<const char *>(e.user.data1),
                           reinterpret_cast<size_t>(e.user.data2));
        term.window.redraw();
        pt.read_complete();
        continue;
      }
      switch (e.type) {
      case SDL_QUIT:
        return;
      case SDL_KEYDOWN: {
        switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          return;
        case SDLK_RETURN:
          pt.write("\n", 1u);
          term.newline();
          break;
        case SDLK_BACKSPACE:
          term.backspace();
          term.window.redraw();
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
        pt.write(input, len);
      } break;
      default: {
      }
      }
    }
  }
}
