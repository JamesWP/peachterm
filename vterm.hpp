#pragma once

#include "graphics.hpp"

namespace app {
class VTerm {
protected:
  // row and column indexes are 0 based.
  int row{0};
  int col{0};

  // scroll end is one past the end row.
  int scroll_row_begin{0};
  int scroll_row_end{23};

  int rows{24};
  int cols{80};

public:
  gfx::TermWin window;

  gfx::TermCell cell;
  gfx::TermCell reset;

  VTerm(int rows, int cols, int pointSize = 14);

  void resize(int rows, int cols, int pointSize);
  void resize(int rows, int cols);

  void overwriteglyph(const char *input, size_t len);
  void putglyph(const char *input, size_t len);

  void curs_newline();
  void curs_backspace();
  void curs_to_col(int col);
  void curs_to_row(int row);

  void cell_set_(gfx::TermCell::Attr attr);
  void cell_reset_(gfx::TermCell::Attr attr);

  template <typename... As> void cell_set(As... as) {
    auto f = [this](auto arg) {
      cell_set_(arg);
      return true;
    };

    bool X[] = {(f(as))...};
    (void)X;
  }

  template <typename... As> void cell_reset(As... as) {
    auto f = [this](auto arg) {
      cell_reset_(arg);
      return true;
    };

    bool X[] = {f(as)...};
    (void)X;
  }

  void cell_set_fg(uint32_t);
  void cell_set_bg(uint32_t);
  // Set the color of the cell

  void scroll_up(int num=1);
  // Scroll the region up, num times

  void scroll_down(int num=1);
  // Scroll the region down, num times

  void insert_lines(int num=1);
  // Insert num lines. scrolls down

  void delete_lines(int num=1);
  // Delete num lines. scrolls up

  void start_new_row();
  // Called after row has been updated. may scroll the screen.

  void move_rows(int top_row, int bottom_row, int rows_up);
  // Move the given set of rows up or down.
};
} // namespace app
