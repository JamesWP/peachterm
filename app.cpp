#include "app.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "io.hpp"
#include "keyboard.hpp"
#include "parser.hpp"

#include <SDL.h>
#include <chrono>
#include <iostream>
#include <string.h>

namespace app {
void App::on_glyph(const char *data, size_t length) {
  putglyph(data, length);
  window.move_cursor(row, col);
#if 0
  window.redraw();
  SDL_Delay(10);
#endif
}

void App::on_backspace() {
  std::cout << "on_backspace\n";
  curs_backspace();
  window.move_cursor(row, col);
}

void App::on_newline() {
  std::cout << "on_newline\n";
  curs_newline();
  window.move_cursor(row, col);
}

void App::on_return() {
  std::cout << "on_return\n";
  curs_to_col(0);
  window.move_cursor(row, col);
}

void App::adjust_cursor(int rows_n, int cols_n) {
  curs_to_col(col + cols_n);
  curs_to_row(row + rows_n);
  window.move_cursor(row, col);
}

void App::perform_el(int arg) {
  switch (arg) {
  case 0: // Erase to right.
    window.clear_cells(row, col, cols);
    return;
  case 1: // Erase to left.
    window.clear_cells(row, 0, col);
    return;
  case 2: // Erase all.
    window.clear_cells(row, 0, cols);
    return;
  }
}

void App::csi_m(const std::vector<int> &args) {
  using A = gfx::TermCell::Attr;

  auto extended_color = [&](auto &i) {
    if (++i == args.end())
      return;

    bool _256 = *i == 5;

    if (++i == args.end())
      return;

    if (_256) {
      col = colors::table[*i];
      return;
    }

    if (++i == args.end())
      return;

    col = *i;
    col <<= 8;

    if (++i == args.end())
      return;

    col |= 0xFF & *i;
    col <<= 8;

    if (++i == args.end())
      return;

    col |= 0xFF & *i;
    col <<= 8;

    col |= 0xFF;
  };

  for (auto i = args.begin(); i != args.end(); i++) {
    int arg = *i;
    bool fg = arg >= 30 && arg < 40;
    uint32_t &col = fg ? cell.fg_col : cell.bg_col;

    // clang-format off
          switch (arg) {
          case 0: 
            cell_reset(A::BOLD, A::ITALIC, A::OVERLINE, A::UNDERLINE, 
                       A::DUNDERLINE, A::STRIKE, A::FEINT, A::REVERSE);
            cell.fg_col = reset.fg_col;
            cell.bg_col = reset.bg_col;
                                                  break; // Reset all
          case 1: cell_set(A::BOLD);              break;
          case 2: cell_set(A::FEINT);             break;
          case 3: cell_set(A::ITALIC);            break; 
          case 4: cell_set(A::UNDERLINE);         break;
          case 7: cell_set(A::REVERSE);           break;
          case 9: cell_set(A::STRIKE);            break; 
          case 21: cell_set(A::DUNDERLINE);       break;
          case 22: cell_reset(A::BOLD, A::FEINT); break;
          case 23: cell_reset(A::ITALIC);         break; 
          case 24: cell_reset(A::UNDERLINE);      break;
          case 27: cell_reset(A::REVERSE);        break;
          case 29: cell_reset(A::STRIKE);         break;
          case 53: cell_set(A::OVERLINE);         break;
          case 55: cell_reset(A::OVERLINE);       break;

          // Font.
          case 10:
            // Reset font.
            break; 
          case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
            // Alternative font. 
            break; 

          // Color.
          case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
          case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
            // Set color. 8 color.
            col = colors::table[arg - (fg?30:40)];
            break; 
          case 38: case 48:
            // Set  color
            // Next arguments are 5;n or 2;r;g;b
            extended_color(i);
            break;
          case 39: case 49:
            // Reset color.
            col = fg ? reset.fg_col : reset.bg_col;
            break; 
          }
    // clang-format on
  }
}

void App::on_csi(char operation, const std::vector<int> &args,
                 std::string_view /*options*/) {
  auto arg = [&](int arg, int def = 0) {
    return ((int)args.size() - 1 > arg) ? args[arg] : def;
  };

  // clang-format off
        switch (operation) {
        case '@': window.insert_cells(row, col, arg(0, 1));    break;
        case 'A': adjust_cursor(-1, 0);                        break;
        case 'B': adjust_cursor(1, 0);                         break;
        case 'C': adjust_cursor(0, 1);                         break;
        case 'D': adjust_cursor(0, -1);                        break;
        case 'E': // -----------------------------------------------;
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J': break; // ----------------------------------------;
        case 'K': perform_el(arg(0));                          break;
        case 'm': if(args.empty()) csi_m({0}); else csi_m(args); break; 
        }
  // clang-format on

  window.move_cursor(row, col);
}

size_t strnlen_s(const char *s, size_t len) {
  for (size_t size = 0; size < len; size++)
    if (s[0] == '\0')
      return size;
    else
      s++;
  return 0;
}

void run() {
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

  std::string pending_input;

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
        std::cout << "SDL Child Data Event\n";
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
        default: {
          pending_input = keyboard::convert_to_input(&e.key);
        }
        }
      } break;
      case SDL_KEYUP: {
        if (!pending_input.empty()) {
          std::cout << "SDL Keypress\n";
          pt.write(pending_input);
          pending_input.clear();
        }
      } break;
      case SDL_TEXTINPUT: {
        char *input = e.text.text;
        size_t len = strnlen_s(input, sizeof(decltype(e.text.text)));
        pt.write(input, len);
        pending_input.clear();
      } break;
      case SDL_MOUSEBUTTONDOWN: {
        std::cout << "Clean redraw\n";
        term.window.dirty();
        term.window.redraw();
      } break;
      default: {
      }
      }
    }
  }
}
} // namespace app
