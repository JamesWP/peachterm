// INTERFACE
#include <string>
#include <vector>
#include <tuple>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;

typedef _TTF_Font TTF_Font;

namespace gfx {

class context {
  bool d_error = false;

public:
  context();
  ~context();

  operator bool() { return !d_error; }

  context(const context &) = delete;
  context &operator=(const context &) = delete;
};

struct TermCell {
  std::string glyph;
  uint32_t fg_col;
  uint32_t bg_col;
  bool bold;
  bool italic;

public:
  friend inline bool operator==(const TermCell &l, const TermCell &r) {
    return std::tie(l.fg_col, l.bg_col, l.bold, l.italic, l.glyph) ==
           std::tie(r.fg_col, r.bg_col, r.bold, r.italic, r.glyph);
  }

  friend inline bool operator!=(const TermCell &l, const TermCell &r) {
    return !(l == r);
  }
};

class TermWin {
  SDL_Window *win;
  SDL_Renderer *ren;
  SDL_Texture *tex;
  TTF_Font *font;

  std::vector<TermCell> cels;
  std::vector<bool> dirty;
  int num_rows;
  int num_cols;
  int cell_width = 6;
  int cell_height = 12;

public:
  TermWin();
  ~TermWin();
  
  TermWin(const TermWin&) = delete;
  TermWin& operator=(const TermWin&) = delete;

  void resize_window(int rows, int cols);
  void set_cell(int row, int col, TermCell cell);
  void redraw();
};

} // namespace gfx

// MAIN FILE
#include <iostream>
#include <chrono>
#include <thread>

template<typename T>
void run(T& t);

int main(int argc, char *argv[]) {
  gfx::context ctx;

  if (!ctx) {
    std::cerr << "Graphics init failed\n";
  }

  gfx::TermWin window;

  window.resize_window(24, 80);
  gfx::TermCell cell;
  cell.fg_col = 0xFFFFFFFF;
  cell.bg_col = 0x000000FF;
  cell.glyph = "H";

  window.set_cell(0, 0, cell);
  window.redraw();

  auto fn = [&window](){ window.redraw(); };
  run(fn); 
}

// GRAPHICS FILE
#include <SDL.h>
#include <SDL_ttf.h>
#include <cairo.h>

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

  font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf", 12);
}

TermWin::~TermWin()
{
  if (tex != nullptr) {
    SDL_DestroyTexture(tex);
    std::cout << "Texture destroyed\n";
  }
  std::cout << "Font destroyed\n";
  TTF_CloseFont(font);
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

  tex = SDL_CreateTexture(ren, 
                          SDL_PIXELFORMAT_RGBA8888, 
                          SDL_TEXTUREACCESS_TARGET, 
                          tex_width,
                          tex_height);
}

void TermWin::set_cell(int row, int col, TermCell cell)
{
  if (row < 0 || col < 0) return;
  if (row >= num_rows) return;
  if (col >= num_cols) return;

  const size_t offset = row * num_cols + col;

  return;
  if (cels[offset] != cell) {
    cels[offset] = cell;
    dirty[offset] = true;
  }
}

void TermWin::redraw()
{ 
  if (tex == nullptr) return;

  SDL_SetRenderTarget(ren, tex);
  SDL_SetRenderDrawColor(ren, 0x77, 0x00, 0xFF, 0xFF);

  for (size_t row = 0; row < num_rows; row++)
  {
    for (size_t col = 0; col < num_cols; col++)
    {
      
    }
  }

#if 1
//  SDL_Rect rect;
//  rect.x = rect.w = 100;
//  rect.y = rect.h = 100;
//  SDL_RenderClear(ren);
//  SDL_RenderFillRect(ren, &rect);
//  SDL_RenderDrawRect(ren, &rect);
#endif

  SDL_SetRenderTarget(ren, nullptr);
  SDL_RenderCopy(ren, tex, nullptr, nullptr);
  SDL_RenderPresent(ren);

  for (size_t i = 0; i < num_cols * num_rows; i++) {
    dirty[i] = 0;
  }
}
} // namespace gfx

template<typename T>
void run(T& t)
{
  SDL_Event e;

  while (true) {
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        return;
      case SDL_KEYDOWN: {
        switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
        case SDLK_q:
          return;
        default: {
        }
        }
      } break;
      default: {
      }
      }

      t();
    }
  }
}
