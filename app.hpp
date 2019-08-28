#pragma once
#include "graphics.hpp"

namespace app {
class VTerm {
protected:
  // row and column indexes are 0 based.
  int row{0};
  int col{0};

  int scroll_row_start{0};
  int scroll_row_end{23};

  int rows{24};
  int cols{80};

public:
  gfx::TermWin window;
  gfx::TermCell cell;

  VTerm(int rows, int cols);

  void overwriteglyph(const char *input, size_t len);
  void putglyph(const char *input, size_t len);

  void curs_newline();
  void curs_backspace();
  void curs_to_col(int col);

  void toggle_bold();
  void toggle_italic();
};

void run();
} // namespace app
