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

  void text_input(const char *input, size_t len);
  void newline();
  void backspace();
};

void run();
} // namespace app
