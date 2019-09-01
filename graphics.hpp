#pragma once
#include <string>
#include <vector>
#include <tuple>

#include "util.hpp"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;
struct SDL_PixelFormat;

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
  std::string glyph = " ";
  uint32_t fg_col = 0xFFFFFFFF;
  uint32_t bg_col = 0x000000FF;
  bool bold = false;
  bool italic = false;
  bool overline = false;
  bool underline = false;
  bool dunderline = false;
  bool strike = false;
  bool feint = false;
  bool reverse = false;

public:
  enum class Attr {
    BOLD,
    ITALIC,
    FG,
    BG,
    OVERLINE,
    UNDERLINE,
    DUNDERLINE,
    STRIKE,
    FEINT,
    REVERSE
  };

  friend inline bool operator==(const TermCell &l, const TermCell &r) {
    return std::tie(l.fg_col, l.bg_col, l.bold, l.italic, l.glyph) ==
           std::tie(r.fg_col, r.bg_col, r.bold, r.italic, r.glyph);
  }

  friend inline bool operator!=(const TermCell &l, const TermCell &r) {
    return !(l == r);
  }
};

enum class Direction { UP, DOWN };

class TermWin {
  SDL_Window *win;
  SDL_Renderer *ren;
  SDL_Texture *tex;

  TTF_Font *fontRegular;
  TTF_Font *fontRegularItalic;
  TTF_Font *fontBold;
  TTF_Font *fontBoldItalic;

  std::vector<util::DirtyTracker<TermCell>> cels;

  int num_rows;
  int num_cols;
  int cell_width = 6;
  int cell_height = 12;
  int font_height = 12;
  int curs_row = 0;
  int curs_col = 0;

public:
  TermWin();
  ~TermWin();
  
  TermWin(const TermWin&) = delete;
  TermWin& operator=(const TermWin&) = delete;

  void resize_window(int rows, int cols);
  void set_cell(int row, int col, TermCell cell);
  void clear_cells(TermCell cell = {});
  void clear_cells(int row, int begin_col, int end_col, TermCell cell = {});
  void insert_cells(int row, int col, int number, TermCell cell = {});
  void redraw();
  void dirty();
  void move_cursor(int row, int col);
  void scroll(int begin_row, int end_row, Direction d, int amount);
};

} // namespace gfx
