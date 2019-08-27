#pragma once
#include "graphics.hpp"

namespace app {
class VTerm {
  int row{0};
  int col{0};
  
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

  void set_fg_red(int red);
  int fg_red() const;
  void set_fg_green(int green);
  int fg_green() const;
  void set_fg_blue(int blue);
  int fg_blue() const;
};

void run();
} // namespace app
