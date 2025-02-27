#pragma once
#include "colors.hpp"
#include "graphics.hpp"
#include "parser.hpp"
#include "vterm.hpp"
#include "io.hpp"
#include "keyboard.hpp"

namespace app {
class App : public parser::VTParser, public app::VTerm {
  io::PseudoTerminal *pt_p;
  keyboard::Mode kMode{keyboard::Mode::Normal};

public:
  App(int rows, int cols, io::PseudoTerminal *pt) : app::VTerm{rows, cols}, pt_p{pt} {}

  // Implement parser::VTParser interface...
  void on_glyph(const char *data, size_t length) override;
  void on_backspace() override;
  void on_newline() override;
  void on_return() override;
  void on_tab() override;
  void on_csi(char operation, const std::vector<int> &args,
              std::string_view /*options*/) override;
  void on_ri() override;
  void on_esc(char op) override;
  void on_osi(int op, std::string_view) override;

  // Helper functions.
  void adjust_cursor(int rows_n, int cols_n);
  void set_cursor(int n_rows, int n_cols);
  void set_scroll_region(int start_row, int end_row);
  void perform_el(int arg);
  void perform_ed(bool selective, int arg);
  void csi_m(const std::vector<int> &args);
  void process_di();
  void process_decset(int arg, bool q);
  void process_decrst(int arg, bool q);
  void process_status_report(int arg);
  keyboard::Mode get_keyboard_mode() const { return kMode; }
};

void run(const gfx::FontSpec&);
} // namespace app
