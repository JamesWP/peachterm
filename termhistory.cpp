#include "termhistory.hpp"

#include <iostream>

TermHistory::TermHistory(std::string filename) {
  // open read and write to enable seek to end
  history_file.open(filename, std::ios_base::in | std::ios_base::out |
                                  std::ios_base::app | std::ios_base::binary);

  if (!history_file) {
    std::cout << "Unable to open history file: " << filename << std::endl;
  }

  std::cout << "Using file for history: " << filename << std::endl;

  start_row();
  gfx::TermCell cell;
  cell.glyph = "H", add_cell_to_history(cell);
  cell.glyph = "e", add_cell_to_history(cell);
  cell.glyph = "l", add_cell_to_history(cell);
  cell.glyph = "l", add_cell_to_history(cell);
  cell.glyph = "o", add_cell_to_history(cell);
  cell.glyph = ".", add_cell_to_history(cell);
  finish_row();
}

void TermHistory::start_row() { buff.clear(); }

void TermHistory::finish_row() {
#ifdef PEACHTERM_IS_VERBOSE
  std::cout << "Row is history  =|";
  for (auto &c : buff) {
    if (c.glyph.size() != 1) {
      std::cout << " ";
    }
    if (!std::isprint(*c.glyph.data())) {
      std::cout << " ";
    }
    std::cout << c.glyph;
  }
  std::cout << std::endl;
#endif

  if (!history_file) {
    return;
  }

  history_file.seekp(0, std::fstream::end);
  size_t trim = 0;
  for (auto i = buff.rbegin(); i != buff.rend(); ++i) {
    // Excluding unicode magic, if the char is a whitespace trim it.
    if (i->glyph.size() == 1 && !iswspace(i->glyph[0])) {
      break;
    }
    if (i->glyph.size() > 1) {
      break;
    }
    trim++;
  }

#ifdef PEACHTERM_IS_VERBOSE
  std::cout << "History line: " << buff.size() << std::endl;
  std::cout << "History line post-trim: " << buff.size() - trim << std::endl;
#endif

  for (size_t i = 0; i < buff.size() - trim; ++i) {
    history_file.write(buff[i].glyph.data(), buff[i].glyph.size());
  }

  history_file << "\n";
  history_file.flush();

  if (!history_file) {
    std::cerr << "Error after writing history file." << std::endl;
  }
}

void TermHistory::add_cell_to_history(const gfx::TermCell &cell) {
  buff.push_back(cell);
}