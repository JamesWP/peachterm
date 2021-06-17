#include "app.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "io.hpp"
#include "keyboard.hpp"
#include "parser.hpp"

#include <SDL.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string.h>

constexpr int user_event_code_stat = 123;
constexpr int user_event_child_data = 124;

namespace {
Uint32 stat_callback(Uint32 interval, void *) {
  std::cout << "Timer callback called\n";
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = user_event_code_stat;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);
  return interval;
}
} // namespace

namespace app {

void App::on_glyph(const char *data, size_t length) {
  putglyph(data, length);
  window.move_cursor(row, col);
  if (getenv("SLOW")) {
    window.redraw();
    SDL_Delay(10);
  }
  if (getenv("WRITE")) {
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

int tab_stop(int start, int num_stops) {
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
  if (row == scroll_row_begin) {
    scroll_down();
  } else {
    adjust_cursor(-1, 0);
  }
}

// TODO: move to vterm
void App::set_scroll_region(int start_row, int end_row) {
  //  scroll_row_* are zero based indexes
  scroll_row_begin = start_row - 1;
  scroll_row_end = end_row; // scroll_row_end is one past end
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

void App::perform_ed(bool selective, int arg) {
  if (selective) {
    return;
  }

  switch (arg) {
  case 0: // Erase below
    window.clear_rows(row + 1, rows);
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
  // return;
  using A = gfx::TermCell::Attr;

  auto parse_extended_colour = [&](auto &i) -> std::optional<uint32_t> {
    ++i;

    if (i == args.end()) {
      // BAD: not enough args
      return {};
    }

    // the first number indicates if we are in 256 color mode (one more number)
    // or full color mode (three more numbers)
    bool _256 = *i == 5;

    ++i;

    if (i == args.end()) {
      // BAD: not enough args
      return {};
    }

    if (_256) {
      return colors::table[*i];
    }

    // read out the R, G, B components (each treated as 8 bit numbers)
    ++i;

    if (i == args.end()) {
      // BAD: not enough args
      return {};
    }

    uint32_t colour = 0xFF & *i;
    colour <<= 8;

    ++i;

    if (i == args.end()) {
      // BAD: not enough args
      return {};
    }

    colour |= 0xFF & *i;
    colour <<= 8;

    ++i;

    if (i == args.end()) {
      // BAD: not enough args
      return {};
    }

    colour |= 0xFF & *i;
    colour <<= 8;

    colour |= 0xFF;

    return colour;
  };

  for (auto i = args.begin(); i != args.end(); ++i) {
    int arg = *i;

    if (arg >= 90 && arg <= 107) {
      arg -= 60;
    }

    bool fg = arg >= 30 && arg < 40;
    uint32_t &colour = fg ? cell.fg_col : cell.bg_col;

    std::optional<uint32_t> extended_colour;

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
            colour = colors::table[arg - (fg?30:40)];
            break;
          case 38: case 48:
            // Set  color
            // Next arguments are 5;n or 2;r;g;b
            extended_colour = parse_extended_colour(i);
            if(extended_colour) {
              colour = *extended_colour;
            break;
            } else {
              return;
            }
          case 39: case 49:
            // Reset color.
            colour = fg ? reset.fg_col : reset.bg_col;
            break;
          }
    // clang-format on
  }
}

void App::process_di() { pt_p->write("\033[65;1;9c"); }

void App::process_decrst(int arg, bool q) {
  if (q) {
    switch (arg) {
    case 1:
      kMode = keyboard::Mode::Normal;
      break;
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
    if (arg == 1047) {
      window.clear_screen();
      printf("Clearing screen buffer\n");
    }
  }
  window.dirty();
}

void App::process_decset(int arg, bool q) {
  if (q) {
    switch (arg) {
    case 1:
      kMode = keyboard::Mode::Application;
      break;
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

void App::process_status_report(int arg) {
  std::ostringstream command_buffer;
  if (arg == 6) {
    command_buffer << "\033[" << col + 1 << ";" << row + 1 << "R";
  } else if (arg == 5) {
    command_buffer << "\033[0n";
  }
  pt_p->write(command_buffer.str());
}

void App::on_osi(int op, std::string_view data) {
  switch (op) {
  case 0: {
    window.set_window_title(data);
  } break;
  }
}

void App::on_csi(char operation, const std::vector<int> &args,
                 std::string_view options) {
  auto arg = [&](int a, int def = 0) {
    return ((int)args.size() > a) ? args[a] : def;
  };
  // Get arg but replace 0's with 1's
  auto ag1 = [&](int a, int def = 0) {
    return arg(a, def) == 0 ? 1 : arg(a, def);
  };

  bool q = options.size() != 0 && options[0] == '?';

  // clang-format off
  switch (operation) {
  case '@': window.insert_cells(row, col, arg(0, 1));    break;
  case 'A': adjust_cursor(-ag1(0, 1), 0);                break;
  case 'B': adjust_cursor(ag1(0, 1), 0);                 break;
  case 'C': adjust_cursor(0, ag1(0, 1));                 break;
  case 'D': adjust_cursor(0, -ag1(0, 1));                break;
  case 'E': set_cursor(row + ag1(0, 1), 0);              break;
  case 'F': set_cursor(row - ag1(0, 1), 0);              break;
  case 'G': set_cursor(row, ag1(0, 1)-1);                break;
  case 'H': set_cursor(ag1(0, 1)-1, ag1(1, 1)-1);        break;
  case 'I': curs_to_col(tab_stop(col, ag1(0, 1)-1));       break;
  case 'J': perform_ed(q, arg(0, 0));                    break;
  case 'K': perform_el(arg(0));                          break;
  case 'L': insert_lines(ag1(0,1));                      break;
  case 'M': delete_lines(ag1(0,1));                      break;

  case 'P': window.delete_cells(row, col, ag1(0, 1));    break;
  case 'S': scroll_up(ag1(0,1));                         break;
  case 'T': scroll_down(ag1(0,1));                       break;

  case 'X': window.clear_cells(row, col, ag1(0,1));      break;
  case 'Z': // -----------------------------------------------;

  case 'c': process_di();                                break;
  case 'f': set_cursor(arg(0, 1)-1, arg(1, 1)-1);        break;
  case 'h': process_decset(arg(0,0),q);                  break;
  case 'l': process_decrst(arg(0,0),q);                  break;
  case 'n': process_status_report(arg(0));               break;
  case 'm': if(args.empty()) csi_m({0}); else csi_m(args); break;
  case 'r': set_scroll_region(ag1(0,1), ag1(1, rows));   break;
  case 's': process_decset(arg(0,0),q);                  break;
  default: std::cout << "UNKNOWN CSI: " << operation << std::endl;   
  }
  // clang-format on

  window.move_cursor(row, col);
}

void App::on_esc(char op) {
  switch (op) {
  case 'D':
    on_newline();
    break;
  case 'E':
    on_newline();
    on_return();
    break;
  }
}

size_t strnlen_s(const char *s, size_t len) {
  for (size_t size = 0; size < len; size++)
    if (s[0] == '\0')
      return size;
    else
      s++;
  return 0;
}

void run(const gfx::FontSpec &spec) {
  SDL_Event data_available;
  SDL_memset(&data_available, 0, sizeof(data_available));
  data_available.type = SDL_USEREVENT;
  data_available.user.code = user_event_child_data;

  std::cout << "PT run\n";

  io::PseudoTerminal pt([&data_available](io::PseudoTerminal *,
                                          const char *data, size_t length) {
    data_available.user.data1 = static_cast<void *>(const_cast<char *>(data));
    data_available.user.data2 = reinterpret_cast<void *>(length);
    SDL_PushEvent(&data_available);
  });

  int rows = 45;
  int cols = 120;

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

  if (!pt.set_size(rows, cols)) {
    std::cerr << "Set size failed\n";
    return;
  }

  std::cout << "App run\n";

  App term{rows, cols, &pt};

  auto hist = std::make_shared<TermHistory>();

  term.window.set_scrollback(hist);

  term.window.load_fonts(spec);

  SDL_Event e;

  // Set callback
  SDL_TimerID timerID = SDL_AddTimer(3 * 1000, stat_callback, &term);

  while (true) {
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        return;
      case SDL_KEYDOWN: {
        switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          if(e.key.keysym.mod & SDLK_LSHIFT) {
            std::cout << "Dump state\n";
            term.window.dump_state_callback();
          } else {
            return;
          }
          break;
        default: {
          pending_input =
              keyboard::convert_to_input(&e.key, term.get_keyboard_mode());
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
      case SDL_WINDOWEVENT: {
        switch (e.window.event) {
        case SDL_WINDOWEVENT_TAKE_FOCUS:
        case SDL_WINDOWEVENT_EXPOSED: {
          term.window.dirty();
          term.window.redraw();
        } break;
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          int width = e.window.data1;
          int height = e.window.data2;

          std::cout << "Window size changed: " << width << "x" << height
                    << std::endl;

          term.window.dirty();
          term.window.redraw();
        } break;
        case SDL_WINDOWEVENT_RESIZED: {
          int width = e.window.data1;
          int height = e.window.data2;

          auto cell_size = term.window.cell_size();
          int new_cols = width / cell_size.first;
          int new_rows = height / cell_size.second;

          std::cout << "Window resized: " << width << "x" << height
                    << std::endl;

          if (abs(new_rows - rows) + abs(new_cols - cols) > 1) {
            term.resize(new_rows, new_cols);
            pt.set_size(new_rows, new_cols);
            std::cout << "Resizing terminal to (" << new_rows << "x "
                      << new_cols << ") after window sizechange" << std::endl;
          }
        } break;
        }
      } break;
      case SDL_USEREVENT: {
        std::cout << "User event\n";
        switch (e.user.code) {
        case user_event_code_stat: {
          std::cout << "Stat event\n";
          term.window.stat_callback();
        } break;
        case user_event_child_data: {
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

        default: {
          std::cout << "Unknown user event code: " << e.user.code << "\n";
        }
        }
      } break;
      default: {
      }
      } // event type switch
    }   // while poll event
    SDL_Delay(10);
  } // while true

  // TODO: this not reached, the above code 'returns' instead of exiting the
  // loop
  SDL_RemoveTimer(timerID);
}
} // namespace app
