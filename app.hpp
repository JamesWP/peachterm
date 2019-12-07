#pragma once
#include "colors.hpp"
#include "graphics.hpp"
#include "parser.hpp"
#include "vterm.hpp"

namespace app {
class App : public parser::VTParser, public app::VTerm {
public:
  App(int rows, int cols) : app::VTerm{rows, cols} {}

  // Implement parser::VTParser interface...
  void on_glyph(const char *data, size_t length) override;
  void on_backspace() override;
  void on_newline() override;
  void on_return() override;
  void on_tab() override;
  void on_csi(char operation, const std::vector<int> &args,
              std::string_view /*options*/) override;

  // Helper functions.
  void adjust_cursor(int rows_n, int cols_n);
  void set_cursor(int n_rows, int n_cols);
  void perform_el(int arg);
  void perform_ed(bool selective, int arg);
  void csi_m(const std::vector<int> &args);
};

void run();
} // namespace app
