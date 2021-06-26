#include <SDL.h>
#include <iostream>
#include <string_view>
#include <tuple>

namespace app {
template <typename T> void check_sdl_ptr_return(T *sdl_return) {
  if (sdl_return == nullptr) {
    const char *sdl_error = SDL_GetError();
    throw std::runtime_error(sdl_error);
  }
}
class TextRenderer {
  SDL_Renderer *_renderer;
  int _row_height;
  int _row_width;
  SDL_Texture *_texture;

public:
  void init(SDL_Renderer *renderer, int row_height, int row_width) {
    _renderer = renderer;
    _row_height = row_height;
    _row_width = row_width;
    _texture =
        SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_TARGET, _row_width, _row_height);
    check_sdl_ptr_return(_texture);
  }

  std::pair<SDL_Texture *, SDL_Rect> render_text(std::string_view text) {
    SDL_Rect row_rect;
    row_rect.w = _row_width;
    row_rect.h = _row_height;
    row_rect.x = 0;
    row_rect.y = 0;
    SDL_SetRenderTarget(_renderer, _texture);
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(_renderer, &row_rect);

    SDL_SetRenderDrawColor(_renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(_renderer, &row_rect);

    SDL_Rect text_rect;
    text_rect.w = 24 + text.size() * 10;
    text_rect.h = _row_height - 10;
    text_rect.y = 5;
    text_rect.x = 10;

    SDL_SetRenderDrawColor(_renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(_renderer, &text_rect);

    return {_texture, row_rect};
  }
};
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

    _curent_row = (_visible_start-vertical_scroll_offset) / row_height;

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

  Terminal _terminal;
  TextRenderer _text_renderer;

  int _vertical_scroll_offset;   // pixels from top of terminal
  int _vertical_scroll_velocity; // pixels per frame

  int _row_height; // pixel height of row
  int _row_width;

public:
  Peachterm() {
    _row_width = 640;
    _row_height = 16;
    _window = SDL_CreateWindow("Peachterm", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, _row_width, 480,
                               SDL_WINDOW_OPENGL);
    check_sdl_ptr_return(_window);
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    check_sdl_ptr_return(_renderer);

    _text_renderer.init(_renderer, _row_height, _row_width);

    _vertical_scroll_velocity = 0;
    _vertical_scroll_offset = 0;
  }
  void event(const SDL_Event &e) {
    switch (e.type) {
    case SDL_MOUSEWHEEL:
      _vertical_scroll_offset += 2 * e.wheel.y;
      _vertical_scroll_velocity += 2*e.wheel.y;
      _vertical_scroll_velocity = std::min(_vertical_scroll_velocity, 100);
      break;
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
    if(std::abs(_vertical_scroll_velocity) > 4) {
      _vertical_scroll_offset += _vertical_scroll_velocity;
    }

    for (auto [screen_row_rect, row_number] :
         RowIterator(window_height, _terminal.get_num_rows(), _row_height,
                     _row_width, _vertical_scroll_offset)) {
      auto [texture, texture_row_rect] =
          _text_renderer.render_text(std::string(std::abs(row_number), 'A'));

      SDL_SetRenderTarget(_renderer, nullptr);
      SDL_RenderCopy(_renderer, texture, &texture_row_rect, &screen_row_rect);
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
  std::cout << "Hello world" << std::endl;
  app::Peachterm app;

  SDL_Event e;

  while (true) {
    app::FrameTimer _frameLimiter;

    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        return 0;
      case SDL_KEYDOWN: {
        switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          return 0;
        }
      } break;
      case SDL_MOUSEWHEEL: {
        if (e.wheel.y != 0) {
          app.event(e);
        }
      } break;
      } // event type switch
    }   // while poll event

    app.render();

  } // while true

  app.close();
  std::cout << "Goodbye world" << std::endl;
}