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

  VTerm();

  void putglyph(const char *input, size_t len);

  void newline();
  void backspace();

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
