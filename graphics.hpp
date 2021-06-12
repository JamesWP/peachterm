#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <string>

#include "util.hpp"
#include "termcell.hpp"
#include "termhistory.hpp"
#include "text_renderer.h"

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

enum class Direction { UP, DOWN };

class TermWin {
  SDL_Window *win = nullptr;
  SDL_Renderer *ren = nullptr;
  SDL_Texture *tex = nullptr;

  TextRenderer tRender;

  bool isNormalScreen = true;

  std::vector<util::DirtyTracker<TermCell>> normalScreen;
  std::vector<util::DirtyTracker<TermCell>> alternativeScreen;

  std::shared_ptr<TermHistory> scrollback;

  int num_rows;
  int num_cols;

  int curs_row = 0;
  int curs_col = 0;

public:
  TermWin(int rows, int cols);
  ~TermWin();

  TermWin(const TermWin &) = delete;
  TermWin &operator=(const TermWin &) = delete;

  void set_scrollback(std::shared_ptr<TermHistory> hist_sp);
  void load_fonts(const FontSpec&);
  // resize grid
  void resize_term(int rows, int cols);
  // resize window
  void auto_resize_window();
  void set_cell(int row, int col, TermCell cell);
  void clear_cells(TermCell cell = {});
  void clear_cells(int row, int begin_col, int end_col, TermCell cell = {});
  void clear_rows(int begin_row, int end_row, TermCell cell = {});
  void clear_screen();
  void insert_cells(int row, int col, int number, TermCell cell = {});
  void delete_cells(int row, int col, int number, TermCell cell = {});
  void redraw();
  void dirty();
  void move_cursor(int row, int col);
  void scroll(int begin_row, int end_row, Direction d, int amount);
  bool& screen_mode_normal();
  std::pair<int, int> cell_size() const;
  void set_window_title(std::string_view);
  void stat_callback();
};

inline bool& TermWin::screen_mode_normal() { return isNormalScreen; }
inline void TermWin::clear_screen() { clear_rows(0, num_rows); }
inline void TermWin::set_scrollback(std::shared_ptr<TermHistory> hist_sp) { this->scrollback = hist_sp; }

} // namespace gfx
