#include "graphics.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <cairo.h>
#include <iostream>
#include <chrono>
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

TermWin::TermWin()
{
  win = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  
  SDL_SetWindowInputFocus(win);

  ren = SDL_CreateRenderer(win, -1, 0);

  tex = nullptr;

  int pointSize = 25;

  fontRegular = TTF_OpenFont(
      "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf", pointSize);

  fontRegularItalic = TTF_OpenFont(
      "/usr/share/fonts/truetype/ubuntu/UbuntuMono-RI.ttf", pointSize);

  fontBold = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
                          pointSize);

  fontBoldItalic = TTF_OpenFont(
      "/usr/share/fonts/truetype/ubuntu/UbuntuMono-BI.ttf", pointSize);

  int advance;
  TTF_GlyphMetrics(fontRegular, 'M', nullptr, nullptr, nullptr, nullptr, &advance);
   
  cell_height = TTF_FontLineSkip(fontRegular);
  cell_width = advance; 
}

TermWin::~TermWin()
{
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

void TermWin::resize_window(int rows, int cols)
{
  if (tex != nullptr) {
    SDL_DestroyTexture(tex);
    std::cout << "Texture destroyed\n";
  }

  num_rows = rows;
  num_cols = cols;

  const int tex_width = num_cols * cell_width;
  const int tex_height = num_rows * cell_height;

  cels.resize(num_rows * num_cols);
  dirty.resize(num_rows * num_cols);

  SDL_SetWindowSize(win, tex_width, tex_height);

  tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_TARGET, tex_width,
                          tex_height);

  std::cout << "TermWin texture resize: " << tex_width << 'x'
            << tex_height << "\n";

  clear_cells();

  redraw();
}

void TermWin::set_cell(int row, int col, TermCell cell)
{
  if (row < 0 || col < 0) return;
  if (row >= num_rows) return;
  if (col >= num_cols) return;

  const size_t offset = row * num_cols + col;

  if (cels[offset] != cell) {
    cels[offset] = cell;
    dirty[offset] = true;
  }
}

void TermWin::clear_cells(TermCell cell) {
  for (int row = 0; row < num_rows; row++) {
    for (int col = 0; col < num_cols; col++) {
       set_cell(row, col, cell);
    }
  }
}

void TermWin::redraw()
{ 
  if (tex == nullptr) return;

  SDL_SetRenderTarget(ren, tex);

  for (int row = 0; row < num_rows; row++) {
    for (int col = 0; col < num_cols; col++) {
      int offset = row * num_cols + col;
      const TermCell &cell = cels[offset];

      if (!dirty[offset])
        continue;

      const char *glyph = cell.glyph.c_str();

      SDL_Color fg, bg;

      fg.r = (cell.fg_col & 0xFF000000) >> 24;
      fg.g = (cell.fg_col & 0x00FF0000) >> 16;
      fg.b = (cell.fg_col & 0x0000FF00) >> 8;
      fg.a = 0xFF;

      bg.r = (cell.bg_col & 0xFF000000) >> 24;
      bg.g = (cell.bg_col & 0x00FF0000) >> 16;
      bg.b = (cell.bg_col & 0x0000FF00) >> 8;
      bg.a = 0xFF;

      TTF_Font *font;
    
      if (!cell.bold && !cell.italic) font = fontRegular;
      if (!cell.bold && cell.italic) font = fontRegularItalic;
      if (cell.bold && !cell.italic) font = fontBold;
      if (cell.bold && cell.italic) font = fontBoldItalic;

      SDL_Surface *cellSurf = TTF_RenderUTF8_Shaded(font, glyph, fg, bg);

      SDL_Texture *cellTex = SDL_CreateTextureFromSurface(ren, cellSurf);

      SDL_Rect cellTex_rect;
      cellTex_rect.x = col * cell_width;
      cellTex_rect.y = row * cell_height;
      cellTex_rect.w = cell_width;
      cellTex_rect.h = cell_height;

      SDL_RenderCopy(ren, cellTex, NULL, &cellTex_rect);

      SDL_DestroyTexture(cellTex);
      SDL_FreeSurface(cellSurf);
    }
  }

  SDL_Rect screen_rect;
  screen_rect.x = screen_rect.y = 0;
  SDL_GetWindowSize(win, &screen_rect.w, &screen_rect.h);

  SDL_SetRenderTarget(ren, nullptr);
  SDL_RenderCopy(ren, tex, &screen_rect, &screen_rect);
  SDL_RenderPresent(ren);

  for (int i = 0; i < num_cols * num_rows; i++) {
    dirty[i] = 0;
  }
}
} // namespace gfx
