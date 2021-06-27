#include <SDL.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <string_view>
#include <tuple>

#include "text_renderer.h"

namespace app {
template <typename T> void check_sdl_ptr_return(T *sdl_return) {
  if (sdl_return == nullptr) {
    const char *sdl_error = SDL_GetError();
    throw std::runtime_error(sdl_error);
  }
}
int round_down_multiple(int value, int multiple) {
  assert(value > 0);
  return value - value % multiple;
}
class RowIterator {
  int _visible_start;
  int _visible_end;

  int _row_height;
  int _row_width;

  int _curent_pos;
  int _curent_row;

public:
  RowIterator(int window_height, int num_rows, int row_height, int row_width,
              int vertical_scroll_offset) {
    int content_start = 0 + vertical_scroll_offset;
    int content_end = num_rows * row_height + vertical_scroll_offset;

    int window_start = -(std::abs(vertical_scroll_offset) % row_height);
    int window_end =
        window_height + std::abs(vertical_scroll_offset) % row_height;

    _visible_start = std::max(content_start, window_start);
    _visible_end = std::min(content_end, window_end);

    _curent_row = (_visible_start - vertical_scroll_offset) / row_height;

    _row_height = row_height;
    _row_width = row_width;

    if (content_end <= window_start || window_end <= content_start) {
      // nothing to see here
      _curent_pos = _visible_end;
    } else {
      _curent_pos = _visible_start;
    }
  }

  RowIterator begin() const { return *this; }

  RowIterator end() const {
    auto n = *this;
    n._curent_pos = _visible_end;
    return n;
  }

  RowIterator &operator++() {

    _curent_pos += _row_height;
    _curent_pos = std::min(_curent_pos, _visible_end);

    _curent_row++;

    return *this;
  }

  bool operator==(const RowIterator &rhs) const {
    return _visible_start == rhs._visible_start &&
           _visible_end == rhs._visible_end && _curent_pos == rhs._curent_pos;
  }

  bool operator!=(const RowIterator &rhs) const { return !(*this == rhs); }

  std::pair<SDL_Rect, int> operator*() const {
    SDL_Rect rect;
    rect.y = _curent_pos;
    rect.x = 0;
    rect.w = _row_width;
    rect.h = _row_height;

    return std::make_pair(rect, _curent_row);
  }
};
class Terminal {
  int rows = 20;

public:
  int get_num_rows() const { return rows; }
};
class Peachterm {
  SDL_Window *_window;
  SDL_Renderer *_renderer;

  // temporary hack
  [[deprecated]] TTF_Font *_font;

  Terminal _terminal;
  std::unique_ptr<gfx::TextRenderer> _text_renderer;

  int _vertical_scroll_offset = 0;   // pixels from top of terminal
  int _vertical_scroll_velocity = 0; // pixels per frame

  static constexpr int initial_window_height = 480;
  static constexpr int initial_window_width = 640;

  static constexpr int initial_cell_width = 10;
  static constexpr int initial_cell_height = 16;
  static constexpr int initial_row_width = initial_window_width;

public:
  Peachterm() {
    _window = SDL_CreateWindow("Peachterm", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, initial_window_width,
                               initial_window_height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    check_sdl_ptr_return(_window);
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    check_sdl_ptr_return(_renderer);

    _text_renderer = std::make_unique<gfx::TextRenderer>(_renderer);

    const std::string font_path =
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";
    _font = TTF_OpenFont(font_path.c_str(), 10);
    check_sdl_ptr_return(_font);
  }
  void event(const SDL_Event &e) {
    switch (e.type) {
    case SDL_MOUSEWHEEL:
      _vertical_scroll_offset += 2 * e.wheel.y;
      _vertical_scroll_velocity += 2 * e.wheel.y;
      _vertical_scroll_velocity = std::min(_vertical_scroll_velocity, 100);
      break;
    case SDL_WINDOWEVENT:
      switch (e.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
        std::cout << "Resized " << e.window.data1 << " x " << e.window.data2
                  << std::endl;
        break;
      default:
        break;
      }
      break;
    default:
      std::cerr << "Peachterm: unknown event type: " << e.type << std::endl;
      assert(false);
    }
  }
  void render() {
    SDL_SetRenderTarget(_renderer, nullptr);
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
    SDL_RenderClear(_renderer);

    int window_height = 0;
    SDL_GetWindowSize(_window, NULL, &window_height);

    // update scroll offset
    _vertical_scroll_velocity *= 0.85;
    if (std::abs(_vertical_scroll_velocity) > 4) {
      _vertical_scroll_offset += _vertical_scroll_velocity;
    }

    SDL_SetRenderTarget(_renderer, nullptr);

    for ([[maybe_unused]] auto [screen_row_rect, row_number] :
         RowIterator(window_height, _terminal.get_num_rows(),
                     _text_renderer->cell_height(), initial_row_width,
                     _vertical_scroll_offset)) {
      const std::string text = "This is row: " + std::to_string(row_number);
      gfx::TextRenderer::CellSpec spec;
      spec.background.a = 255;
      spec.background.r = spec.background.g = spec.background.b = 255;

      spec.foreground.a = 255;
      spec.foreground.r = spec.foreground.g = spec.foreground.b = 0;

      spec.font = _font;

      SDL_Rect screen_char_rect;
      screen_char_rect.x = screen_row_rect.x;
      screen_char_rect.y = screen_row_rect.y;

      for (auto c : text) {
        std::string c_str;
        c_str.append(&c, 1u);
        spec.glyph = c_str;

        auto [char_rect, char_texture] = _text_renderer->draw_character(spec);
        screen_char_rect.w = char_rect.w;
        screen_char_rect.h = char_rect.h;

        SDL_RenderCopy(_renderer, char_texture, &char_rect, &screen_char_rect);

        screen_char_rect.x += char_rect.w - 1;
      }
    }

    SDL_RenderPresent(_renderer);
  }
  void close() {
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
  }
};
class FrameTimer {
  // 1 tick is 1ms, time each frame must take minimum
  static constexpr int FRAME_TICKS_MIN = 1000 / 60;
  const unsigned int _frameStartTicks;

public:
  FrameTimer() : _frameStartTicks{SDL_GetTicks()} {}
  ~FrameTimer() {
    const unsigned int frameEndTicks = SDL_GetTicks();
    const unsigned int frameTimeTicks = frameEndTicks - _frameStartTicks;
    const int remainingFrameTicks =
        FRAME_TICKS_MIN - static_cast<int>(frameTimeTicks);
    if (remainingFrameTicks > 0) {
      SDL_Delay(remainingFrameTicks);
    }
  }
};
} // namespace app

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  TTF_Init();

  std::cout << "Hello world" << std::endl;
  {
    app::Peachterm app;

    SDL_Event e;

    bool running = true;

    while (running) {
      app::FrameTimer _frameLimiter;

      while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_KEYDOWN: {
          switch (e.key.keysym.sym) {
          case SDLK_ESCAPE:
            running = false;
          }
        } break;
        case SDL_MOUSEWHEEL: {
          if (e.wheel.y != 0) {
            app.event(e);
          }
        } break;
        case SDL_WINDOWEVENT: {
          switch (e.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            e.type = SDL_QUIT;
            SDL_PushEvent(&e);
            break;
          case SDL_WINDOWEVENT_RESIZED:
            app.event(e);
            break;
          }
          break;
        }
        } // event type switch
      }   // while poll event

      app.render();

    } // while true

    app.close();
  }
  std::cout << "Goodbye world" << std::endl;
}