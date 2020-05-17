#include "app.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "io.hpp"
#include "keyboard.hpp"
#include "parser.hpp"

#include <SDL.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string.h>


namespace app {
void App::on_glyph(const char *data, size_t length) {
  putglyph(data, length);
  window.move_cursor(row, col);
  if(getenv("SLOW")) {
    window.redraw();
    SDL_Delay(10);
  }
  if(getenv("WRITE")) {
    std::string_view glyph{data, length};
    std::cout << "Data: " << std::quoted(glyph) << "\n";
  }
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

int tab_stop(int start, int num_stops)
{
  return start + 8 * num_stops - (start & 7);
}

void App::on_tab() {
  std::cout << "on_tab\n";
  curs_to_col(tab_stop(col, 1));
  window.move_cursor(row, col);
}

void App::adjust_cursor(int rows_n, int cols_n) {
  curs_to_row(row + rows_n);
  curs_to_col(col + cols_n);
  window.move_cursor(row, col);
}

void App::set_cursor(int n_row, int n_col) {
  curs_to_row(n_row);
  curs_to_col(n_col);
  window.move_cursor(row, col);
}

void App::on_ri() {
  // if at top of window, scroll down
  if(row == scroll_row_begin){
    scroll_down();
  } else {
    adjust_cursor(-1, 0);
  }
}

// TODO: move to vterm
void App::set_scroll_region(int start_row, int end_row) {
  scroll_row_begin = start_row-1;
  scroll_row_end = end_row-1;
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

void App::perform_ed(bool selective, int arg)
{
  if (selective) {
    return;
  }

  switch (arg) {
  case 0: // Erase below
    window.clear_rows(row+1, rows);
    break;
  case 1: // Erase above
    window.clear_rows(0, row);
    break;
  case 2: // Erase all
    window.clear_rows(0, rows);
    break;
  case 3: // Erase saved lines
  default:
    break;
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

void App::process_di() { pt_p->write("\e[65;1;9c"); }

void App::process_decrst(int arg, bool q) {
  if(q) {
    switch(arg) {
    case 3:
      resize(rows, 80);
      break;
    case 47:
    case 1047:
    case 1049:
      window.screen_mode_normal() = true;
      printf("Normal screen buffer\n");
      break;
    }
    if(arg == 1047) {
      window.clear_screen();
      printf("Clearing screen buffer\n");
    }
  }
  window.dirty();
}

void App::process_decset(int arg, bool q) {
  if(q) {
    switch(arg) {
    case 47:
    case 1047:
    case 1049:
      window.screen_mode_normal() = false;
      printf("Alternate screen buffer\n");
    }
    if (arg == 1049) {
      window.clear_screen();
      printf("Clearing screen buffer\n");
    }
  } else {
    
  }
  window.dirty();
}

void App::on_csi(char operation, const std::vector<int> &args,
                 std::string_view options) {
  auto arg = [&](int arg, int def = 0) {
    return ((int)args.size() > arg) ? args[arg] : def;
  };

  bool q = options.size() != 0 && options[0] == '?';

  // clang-format off
  switch (operation) {
  case '@': window.insert_cells(row, col, arg(0, 1));    break;
  case 'A': adjust_cursor(-arg(0, 1), 0);                break;
  case 'B': adjust_cursor(arg(0, 1), 0);                 break;
  case 'C': adjust_cursor(0, arg(0, 1));                 break;
  case 'D': adjust_cursor(0, -arg(0, 1));                break;
  case 'E': set_cursor(row + arg(0, 1), 0);              break;
  case 'F': set_cursor(row - arg(0, 1), 0);              break;
  case 'G': set_cursor(row, arg(0, 1)-1);                break;
  case 'H': set_cursor(arg(0, 1)-1, arg(1, 1)-1);        break;
  case 'I': curs_to_col(tab_stop(col, arg(0, 1)));       break;
  case 'J': perform_ed(q, arg(0, 0));                    break;
  case 'K': perform_el(arg(0));                          break;
  case 'L': insert_lines(arg(0,1));                      break;
  case 'M': delete_lines(arg(0,1));                      break;

  case 'P': window.delete_cells(row, col, arg(0, 1));    break;
  case 'S': scroll_up(arg(0,1));                         break;
  case 'T': scroll_down(arg(0,1));                       break;

  case 'X': // -----------------------------------------------;
  case 'Z': // -----------------------------------------------;

  case 'c': process_di();                                break;
  case 'f': set_cursor(arg(0, 1)-1, arg(1, 1)-1);        break;
  case 'h': process_decset(arg(0,0),q);                  break;
  case 'l': process_decrst(arg(0,0),q);                  break;
  case 'n': /* see eduterm */                            break;
  case 'm': if(args.empty()) csi_m({0}); else csi_m(args); break; 
  case 'r': set_scroll_region(arg(0,1), arg(1, rows));   break;
  case 's': /* see eduterm */                            break;
  case 't': /* see eduterm */                            break;
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
  int pointSize = 12;

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

  App term{rows, cols, pointSize, &pt};

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
    SDL_Delay(16.60);
  }
}
} // namespace app
