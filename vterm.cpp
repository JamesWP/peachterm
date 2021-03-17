#include "vterm.hpp"

namespace app {
VTerm::VTerm(int _rows, int _cols, int pointSize)
: window(_rows, _cols, pointSize)
{
  cell.fg_col = 0xFFFFFFFF;
  cell.bg_col = 0x000000FF;
}

void VTerm::resize(int _rows, int _cols, int pointSize) {
  rows = _rows;
  cols = _cols;
  window.load_fonts(pointSize);
  window.resize_window(rows, cols);
}

void VTerm::overwriteglyph(const char *input, size_t len) {
  cell.glyph.assign(input, len);
  window.set_cell(row, col, cell);
}

void VTerm::start_new_row() {
  // If the next row has put us beyond the scroll region:
  if (row == scroll_row_end + 1) {
    // scroll up and start the last line again.
    window.scroll(scroll_row_begin, scroll_row_end+1, gfx::Direction::UP, 1);
    row = scroll_row_end;
  }
}

void VTerm::insert_lines(int num) {
  scroll_down(num);
}

void VTerm::delete_lines(int num) {
  scroll_up(num);
}

void VTerm::scroll_up(int num) {
  // scroll [row, scroll_row_end] up num times
  window.scroll(row, scroll_row_end+1, gfx::Direction::UP, num);
}

void VTerm::scroll_down(int num) {
  // scroll [row, scroll_row_end] down num times
  window.scroll(row, scroll_row_end+1, gfx::Direction::DOWN, num);
}

void VTerm::putglyph(const char *input, size_t len) {

  // If we are of the rightmost column, we start the next row.
  if (col >= cols) {
    col = 0;
    row++;

    start_new_row();
  }

  overwriteglyph(input, len);

  col++;
}

void clamp(int &v, int min, int max) {
  if (v > max)
    v = max;
  else if (v < min)
    v = min;
}

void curs_clamp(int &row, int &col, int rows, int cols) {
  clamp(row, 0, rows-1);
  clamp(col, 0, cols-1);
}

void VTerm::curs_newline() {
  row++;
  col = 0;

  start_new_row();

  curs_clamp(row, col, rows, cols);
}

void VTerm::curs_to_col(int _col) {
  col = _col;

  curs_clamp(row, col, rows, cols);
}

void VTerm::curs_to_row(int _row) {
  row = _row;

  curs_clamp(row, col, rows, cols);
}

void VTerm::curs_backspace() {
  col--;

  curs_clamp(row, col, rows, cols);
}

bool &get_attr(gfx::TermCell &cell, gfx::TermCell::Attr attr) {
  static bool b = false;
  using Attr = gfx::TermCell::Attr;
  // clang-format off
  switch (attr) {
  case Attr::BOLD:       return cell.bold;
  case Attr::ITALIC:     return cell.italic;
  case Attr::OVERLINE:   return cell.overline;
  case Attr::UNDERLINE:  return cell.underline;
  case Attr::DUNDERLINE: return cell.dunderline;
  case Attr::STRIKE:     return cell.strike;
  case Attr::FEINT:      return cell.feint;
  case Attr::REVERSE:    return cell.reverse;
  case Attr::FG: case Attr::BG: default: return b;
  }
  // clang-format on
}

void VTerm::cell_set_(gfx::TermCell::Attr attr) { get_attr(cell, attr) = true; }

void VTerm::cell_reset_(gfx::TermCell::Attr attr) {
  get_attr(cell, attr) = get_attr(reset, attr);
}

} // namespace app
