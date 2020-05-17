#include "graphics.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
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

TermWin::TermWin(int rows = 24, int cols = 80, int pointSize = 10) {
  this->load_fonts(pointSize);

  const int window_width = cols * cell_width;
  const int window_height = rows * cell_height;

  win = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, window_width, window_height, 
                         0);

  SDL_SetWindowInputFocus(win);

  ren = SDL_CreateRenderer(win, -1, 0);

  tex = nullptr;

  this->resize_window(rows, cols);
}

TermWin::~TermWin() {
  if (tex != nullptr) {
    SDL_DestroyTexture(tex);
    std::cout << "Texture destroyed\n";
  }
  std::cout << "Font destroyed\n";
  TTF_CloseFont(fontRegular);
  std::cout << "Renderer destroyed\n";
  SDL_DestroyRenderer(ren);
  std::cout << "Window destroyed\n";
  SDL_DestroyWindow(win);
}

void TermWin::load_fonts(int pointSize) {
  if(fontRegular!=0) {
    TTF_CloseFont(fontRegular);
  }
  fontRegular = TTF_OpenFont(
      "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf", pointSize);
  //fontRegular = TTF_OpenFont(
  //      "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf", pointSize);

  if(fontRegularItalic!=0) {
    TTF_CloseFont(fontRegularItalic);
  }
  fontRegularItalic = TTF_OpenFont(
      "/usr/share/fonts/truetype/ubuntu/UbuntuMono-RI.ttf", pointSize);

  if(fontBold!=0) {
    TTF_CloseFont(fontBold);
  }
  fontBold = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
                          pointSize);

  if(fontBoldItalic!=0) {
    TTF_CloseFont(fontBoldItalic);
  }
  fontBoldItalic = TTF_OpenFont(
      "/usr/share/fonts/truetype/ubuntu/UbuntuMono-BI.ttf", pointSize);

  std::cout << fontRegular << fontRegularItalic << fontBold << fontBoldItalic 
            << std::endl;

  int advance;
  TTF_GlyphMetrics(fontRegular, '&', nullptr, nullptr, nullptr, nullptr,
                   &advance);

  font_height = TTF_FontHeight(fontRegular);

  // if (font_height>30) { font_height = 30; }
  // if(advance>15) {advance = 15; } 

  cell_height = font_height;
  cell_width = advance;
}

void TermWin::resize_window(int rows, int cols) {
  if (tex != nullptr) {
    SDL_DestroyTexture(tex);
    std::cout << "Texture destroyed\n";
  }

  num_rows = rows;
  num_cols = cols;

  const int tex_width = num_cols * cell_width;
  const int tex_height = num_rows * cell_height;

  normalScreen.resize(num_rows * num_cols);
  alternativeScreen.resize(num_rows * num_cols);

  //TODO: check if already this size
  std::cout << "width:" << tex_width << " height:" << tex_height << std::endl;
  SDL_SetWindowSize(win, tex_width, tex_height);

  tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_TARGET, tex_width, tex_height);

  std::cout << "TermWin texture resize: " << tex_width << 'x' << tex_height
            << "\n";

  clear_cells();

  redraw();
}

void TermWin::set_cell(int row, int col, TermCell cell) {
  if (row < 0 || col < 0)
    return;
  if (row >= num_rows)
    return;
  if (col >= num_cols)
    return;

  const size_t offset = row * num_cols + col;

  if(isNormalScreen) {
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
  for (int col = begin_col; col < end_col; col++) {
    set_cell(row, col, cell);
  }
}

void TermWin::clear_rows(int begin_row, int end_row, TermCell cell) {
  for (int row = begin_row; row != end_row; row++) {
    clear_cells(row, 0, num_cols, cell);
  }
}

void TermWin::insert_cells(int row, int col, int number, TermCell cell) {
  auto& cels = isNormalScreen ? normalScreen : alternativeScreen;
  auto begin = cels.begin() + row * num_cols + col;
  auto end = cels.begin() + (row + 1) * num_cols;
  std::rotate(begin, end - number, end);
  clear_cells(row, col, col + number, cell);
}

void TermWin::delete_cells(int row, int col, int number, TermCell cell) {
  auto& cels = isNormalScreen ? normalScreen : alternativeScreen;
  auto begin = cels.begin() + row * num_cols + col;
  auto end = cels.begin() + (row + 1) * num_cols;
  std::rotate(begin, begin + number, end);
  clear_cells(row, num_cols - number, num_cols, cell);
}

void TermWin::dirty() {
  auto& cels = isNormalScreen ? normalScreen : alternativeScreen;
  for (auto &c : cels) {
    c.dirty() = true;
  }
}

void TermWin::redraw() {
  auto& cels = isNormalScreen ? normalScreen : alternativeScreen;
  if (tex == nullptr)
    return;

  SDL_SetRenderTarget(ren, tex);

  for (int row = 0; row < num_rows; row++) {
    for (int col = 0; col < num_cols; col++) {
      // Cell locaiton.
      int offset = row * num_cols + col;
      const TermCell &cell = cels[offset].value();
      bool dirty = cels[offset].dirty();
      int cell_top_y = row * cell_height;
      int cell_left_x = col * cell_width;

      // Cell content.
      const char *glyph = cell.glyph.c_str();
  
      // Cell color.
      SDL_Color fg, bg;
      fg.r = (cell.fg_col & 0xFF000000) >> 24;
      fg.g = (cell.fg_col & 0x00FF0000) >> 16;
      fg.b = (cell.fg_col & 0x0000FF00) >> 8;
      fg.a = 0xFF;
      bg.r = (cell.bg_col & 0xFF000000) >> 24;
      bg.g = (cell.bg_col & 0x00FF0000) >> 16;
      bg.b = (cell.bg_col & 0x0000FF00) >> 8;
      bg.a = 0xFF;

      // Cursor.
      bool is_cursor = row == curs_row && col == curs_col;
      int curs_height = 2;

      // Rectangle for the cell.
      SDL_Rect cell_rect;
      cell_rect.w = cell_width;
      cell_rect.h = cell_height;
      cell_rect.x = cell_left_x;
      cell_rect.y = cell_top_y;

      // Rectangle for the cursor.
      SDL_Rect curs_rect;
      curs_rect.w = cell_width;
      curs_rect.h = curs_height;
      curs_rect.x = cell_left_x;
      curs_rect.y = cell_top_y + cell_height - curs_height;

      // Cell font;
      TTF_Font *font;
      if (!cell.bold && !cell.italic)
        font = fontRegular;
      if (!cell.bold && cell.italic)
        font = fontRegularItalic;
      if (cell.bold && !cell.italic)
        font = fontBold;
      if (cell.bold && cell.italic)
        font = fontBoldItalic;

      // And now, actual drawing.

      // Don't draw an unchanged cell, ..., unless it's the cursor.
      if (!dirty && !is_cursor)
        continue;

      if (cell.reverse) {
        std::swap(fg, bg);
      }

      // Clear cursor.
      SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, 0xFF);
      SDL_RenderFillRect(ren, &curs_rect);

      // Begin draw character.
      SDL_Surface *cellSurf = TTF_RenderUTF8_Blended(font, glyph, fg);
      SDL_Texture *cellTex = SDL_CreateTextureFromSurface(ren, cellSurf);
      SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, 0xFF);
      SDL_RenderFillRect(ren, &cell_rect);
      SDL_RenderCopy(ren, cellTex, NULL, &cell_rect);
      SDL_DestroyTexture(cellTex);
      SDL_FreeSurface(cellSurf);

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

  SDL_SetRenderTarget(ren, nullptr);

  SDL_RenderCopy(ren, tex, &screen_rect, &screen_rect);

  SDL_RenderPresent(ren);
}

void TermWin::move_cursor(int row, int col) {
  curs_row = row;
  curs_col = col;
}

void TermWin::scroll(int begin_row, int end_row, Direction d, int amount) {
  auto& cels = isNormalScreen ? normalScreen : alternativeScreen;

  std::cout << "Scrolling rows " << begin_row << " - " << end_row << " ";
  std::cout << (d == Direction::UP ? "UP" : "DOWN") << " by " << amount << "\n";

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
    for (auto b = mid; b != end; b++) {
      *b = clear;
    }
  } else {
    // blank lines are at the start.
    // clear start - mid.
    auto begin = row_it(begin_row);
    for (auto b = begin; b != mid; b++) {
      *b = clear;
    }
  }
}

} // namespace gfx
