#include "app.hpp"
#include "graphics.hpp"
#include "io.hpp"
#include "parser.hpp"

#include <SDL.h>
#include <chrono>
#include <iostream>
#include <string.h>

namespace app {
VTerm::VTerm(int _rows, int _cols) : rows{_rows}, cols{_cols} {
  window.resize_window(rows, cols);
  cell.fg_col = 0xFFFFFFFF;
  cell.bg_col = 0x000000FF;
}

void VTerm::overwriteglyph(const char *input, size_t len) {
  cell.glyph.assign(input, len);
  window.set_cell(row, col, cell);
}

void VTerm::putglyph(const char *input, size_t len) {
  overwriteglyph(input, len);

  col++;

  if (col >= cols) {
    col = 0;
    row++;
  }

  if (row >= rows) {
    row = rows - 1;
  }
}

void clamp(int& v, int min, int max) {
  if (v>max) v = max;
  else if (v < min) v = min;
}

void curs_clamp(int& row, int& col, int rows, int cols) {
  clamp(row, 0, rows);
  clamp(col, 0, cols);
}

void VTerm::curs_newline() {
  row++;
  col = 0;

  curs_clamp(row, col, rows, cols);
}

void VTerm::curs_to_col(int col) {
  col = 0;

  curs_clamp(row, col, rows, cols);
}

void VTerm::curs_backspace() {
  col--;

  curs_clamp(row, col, rows, cols);
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
      App(int rows, int cols) : app::VTerm{rows, cols} {}

      void on_glyph(const char *data, size_t length) override {
        std::cout << "on_glyph length: " << length << "\n";
        putglyph(data, length);
        window.move_cursor(row, col);
      }

      void on_backspace() override {
        std::cout << "on_backspace\n";
        curs_backspace();
        window.move_cursor(row, col);
      }

      void on_newline() override {
        std::cout << "on_newline\n";
        curs_newline();
        window.move_cursor(row, col);
      }

      void on_return() override {
        std::cout << "on_return\n";
        curs_to_col(0);
        window.move_cursor(row, col);
      }

      void on_csi_K(const char *, size_t) override {
        char space = ' ';
        overwriteglyph(&space, 1u);
      }
};

void app::run()
{
  const uint32_t data_available_event = SDL_RegisterEvents(1);

  SDL_Event data_available;
  SDL_memset(&data_available, 0, sizeof(data_available));
  data_available.type = data_available_event;

  std::cout << "PT run\n";

  io::PseudoTerminal pt([&data_available](io::PseudoTerminal *,
                                          const char *data, size_t length) {
    data_available.user.data1 = static_cast<void *>(const_cast<char *>(data));
    data_available.user.data2 = reinterpret_cast<void *>(length);
    SDL_PushEvent(&data_available);
  });

  int rows = 24;
  int cols = 80;

  if (!pt.start()) {
    std::cerr << "Start failed\n";
    return;
  }

  if (!pt.set_size(rows, cols)) {
    std::cerr << "Set size failed\n";
    return;
  }

  if (!pt.fork_child()) {
    std::cerr << "Fork child failed\n";
    return;
  }

  std::cout << "App run\n";

  App term{rows, cols};

  SDL_Event e;

  while (true) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == data_available_event) {
        std::cout << "SDL Event\n";
        const char *data = static_cast<const char *>(e.user.data1);
        size_t len = reinterpret_cast<size_t>(e.user.data2);

        if (data == nullptr && len == 0u) {
          return;
        }

        term.parse_input(data, len);
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
          pt.write('\n');
          break;
        case SDLK_BACKSPACE:
          pt.write('\b');
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
