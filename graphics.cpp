#include "graphics.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

namespace gfx {

context::context() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    d_error = true;
  }

  if (TTF_Init() < 0) {
    d_error = true;
  }
}

context::~context() {
  std::cout << "SDL quit\n";
  SDL_Quit();
}

TermWin::TermWin(int rows = 24, int cols = 80) {
  const int window_width = cols * tRender.cell_width();
  const int window_height = rows * tRender.cell_height();

  win = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, window_width, window_height,
                         SDL_WINDOW_RESIZABLE);

  SDL_SetWindowInputFocus(win);

  ren = SDL_CreateRenderer(win, -1, 0);

  tex = nullptr;

  this->resize_term(rows, cols);
}

TermWin::~TermWin() {
  if (tex != nullptr) {
    SDL_DestroyTexture(tex);
    std::cout << "Texture destroyed\n";
  }
  std::cout << "Renderer destroyed\n";
  SDL_DestroyRenderer(ren);
  std::cout << "Window destroyed\n";
  SDL_DestroyWindow(win);
}

void TermWin::load_fonts(const FontSpec &spec) {
  tRender.load_fonts(ren, spec);
}

void TermWin::resize_term(int rows, int cols) {
  std::cout << "Size:" << rows << " rows by " << cols << " cols" << std::endl;
  if (tex != nullptr) {
    SDL_DestroyTexture(tex);
    std::cout << "Texture destroyed\n";
  }

  num_rows = rows;
  num_cols = cols;

  const int tex_width = num_cols * tRender.cell_width();
  const int tex_height = num_rows * tRender.cell_height();

  normalScreen.resize(num_rows * num_cols);
  alternativeScreen.resize(num_rows * num_cols);

  tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_TARGET, tex_width, tex_height);

  std::cout << "TermWin texture resize: " << tex_width << 'x' << tex_height
            << "\n";

  clear_cells();
}

void TermWin::auto_resize_window() {
  SDL_Rect texture_rect;
  texture_rect.x = texture_rect.y = 0;
  SDL_QueryTexture(tex, nullptr, nullptr, &texture_rect.w, &texture_rect.h);

  SDL_Rect window_rect;
  window_rect.x = window_rect.y = 0;
  SDL_GetWindowSize(win, &window_rect.w, &window_rect.h);

  // check if already this size
  if (window_rect.w == texture_rect.w && window_rect.h == texture_rect.h) {
    return;
  }

  std::cout << "width:" << texture_rect.w << " height:" << texture_rect.h
            << std::endl;
  SDL_SetWindowSize(win, texture_rect.w, texture_rect.h);
}

void TermWin::set_cell(int row, int col, TermCell cell) {
  if (row < 0 || col < 0)
    return;
  if (row >= num_rows)
    return;
  if (col >= num_cols)
    return;

  const size_t offset = row * num_cols + col;

  if (isNormalScreen) {
    normalScreen[offset] = cell;
  } else {
    alternativeScreen[offset] = cell;
  }
}

void TermWin::clear_cells(TermCell cell) {
  for (int row = 0; row < num_rows; row++) {
    for (int col = 0; col < num_cols; col++) {
      set_cell(row, col, cell);
    }
  }
}

void TermWin::clear_cells(int row, int begin_col, int end_col, TermCell cell) {
  for (int col = begin_col; col < end_col && col < num_cols; col++) {
    set_cell(row, col, cell);
  }
}

void TermWin::clear_rows(int begin_row, int end_row, TermCell cell) {
  for (int row = begin_row; row != end_row; row++) {
    clear_cells(row, 0, num_cols, cell);
  }
}

void TermWin::insert_cells(int row, int col, int number, TermCell cell) {
  auto &cels = isNormalScreen ? normalScreen : alternativeScreen;
  auto begin = cels.begin() + row * num_cols + col;
  auto end = cels.begin() + (row + 1) * num_cols;
  std::rotate(begin, end - number, end);
  clear_cells(row, col, col + number, cell);
}

void TermWin::delete_cells(int row, int col, int number, TermCell cell) {
  auto &cels = isNormalScreen ? normalScreen : alternativeScreen;
  auto begin = cels.begin() + row * num_cols + col;
  auto end = cels.begin() + (row + 1) * num_cols;
  std::rotate(begin, begin + number, end);
  clear_cells(row, num_cols - number, num_cols, cell);
}

void TermWin::dirty() {
  auto &cels = isNormalScreen ? normalScreen : alternativeScreen;
  for (auto &c : cels) {
    c.dirty() = true;
  }
}

void TermWin::redraw() {
  auto &cels = isNormalScreen ? normalScreen : alternativeScreen;
  if (tex == nullptr)
    return;

  SDL_SetRenderTarget(ren, tex);

  for (int row = 0; row < num_rows; row++) {
    for (int col = 0; col < num_cols; col++) {
      // Cell locaiton.
      int offset = row * num_cols + col;
      const TermCell &cell = cels[offset].value();
      bool dirty = cels[offset].dirty();
      int cell_top_y = row * tRender.cell_height();
      int cell_left_x = col * tRender.cell_width();

      // Cell content.
      const char *glyph = cell.glyph.c_str();

      // Cell color.
      SDL_Color fg, bg;
      fg.r = static_cast<uint8_t>((cell.fg_col & 0xFF000000) >> 24);
      fg.g = static_cast<uint8_t>((cell.fg_col & 0x00FF0000) >> 16);
      fg.b = static_cast<uint8_t>((cell.fg_col & 0x0000FF00) >> 8);
      fg.a = 0xFF;
      bg.r = static_cast<uint8_t>((cell.bg_col & 0xFF000000) >> 24);
      bg.g = static_cast<uint8_t>((cell.bg_col & 0x00FF0000) >> 16);
      bg.b = static_cast<uint8_t>((cell.bg_col & 0x0000FF00) >> 8);
      bg.a = 0xFF;

      // Cursor.
      bool is_cursor = row == curs_row && col == curs_col;
      int curs_height = 2;

      // Rectangle for the cursor.
      SDL_Rect curs_rect;
      curs_rect.w = tRender.cell_width();
      curs_rect.h = curs_height;
      curs_rect.x = cell_left_x;
      curs_rect.y = cell_top_y + tRender.cell_height() - curs_height;

      // Cell font;
      TTF_Font *font = tRender.get_font(cell.bold, cell.italic);

      // And now, actual drawing.

      // Don't draw an unchanged cell, ..., unless it's the cursor.
      if (!dirty && !is_cursor)
        continue;

      if (cell.reverse) {
        std::swap(fg, bg);
      }

      tRender.draw_character(ren, font, glyph, fg, bg, cell_top_y, cell_left_x);

      if (is_cursor) {
        // Begin draw cursor.
        SDL_SetRenderDrawColor(ren, fg.r, fg.g, fg.b, 0xFF);
        SDL_RenderFillRect(ren, &curs_rect);
        cels[offset].dirty() = true;
      } else {
        cels[offset].dirty() = false;
      }
    }
  }

  SDL_Rect screen_rect;
  screen_rect.x = screen_rect.y = 0;
  SDL_GetWindowSize(win, &screen_rect.w, &screen_rect.h);

  SDL_Rect texture_rect;
  texture_rect.x = texture_rect.y = 0;
  SDL_QueryTexture(tex, nullptr, nullptr, &texture_rect.w, &texture_rect.h);

  SDL_SetRenderTarget(ren, nullptr);

  SDL_RenderCopy(ren, tex, &texture_rect, &texture_rect);

  SDL_RenderPresent(ren);
}

void TermWin::move_cursor(int row, int col) {
  curs_row = row;
  curs_col = col;
}

// range is: [begin_row, end_row)
void TermWin::scroll(int begin_row, int end_row, Direction d, int amount) {
  auto &cels = isNormalScreen ? normalScreen : alternativeScreen;

#ifdef PEACHTERM_IS_VERBOSE
  std::cout << "Scrolling rows [" << begin_row << ", " << end_row << ") "
            << (d == Direction::UP ? "UP" : "DOWN") << " by " << amount << "\n";
#endif

  auto row_it = [&](int row) { return cels.begin() + num_cols * row; };

  auto mid = std::rotate(row_it(begin_row),
                         d == Direction::UP ? row_it(begin_row + amount)
                                            : row_it(end_row - amount),
                         row_it(end_row));

  TermCell clear;

  if (d == Direction::UP) {
    // blank lines are at the end.
    // clear mid - end.
    auto end = row_it(end_row);
    for (auto b = mid; b != end; b += num_cols) {
      if (scrollback) {
        scrollback->add_row_to_history(b, b + num_cols);
      }
    }
    for (auto b = mid; b != end; b++) {
      *b = clear;
    }
  } else {
    // blank lines are at the start.
    // clear start - mid.
    auto begin = row_it(begin_row);
    for (auto b = begin; b != mid; b += num_cols) {
      if (scrollback) {
        scrollback->add_row_to_history(b, b + num_cols);
      }
    }
    for (auto b = begin; b != mid; b++) {
      *b = clear;
    }
  }
}

std::pair<int, int> TermWin::cell_size() const {
  return tRender.cell_size();
}

void TermWin::set_window_title(std::string_view data) {
  SDL_SetWindowTitle(win, data.data());
}

void TermWin::stat_callback() {
  tRender.dump_cache_stats();
}

void TermWin::dump_state_callback() {
  tRender.dump_cache_to_disk(ren);
}
} // namespace gfx
