#include "fonts.hpp"
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  auto font_manager = fonts::Manager();

  auto default_font = font_manager.defaultFont();
  if (default_font) {
    std::cout << "Listing default font: " << default_font.value() << std::endl;
  } else {
    std::cout << "Listing default font: NONE!" << std::endl;
  }

  auto families = font_manager.familyList();

  const std::vector<fonts::Style> styles = {
      fonts::Style::Regular, fonts::Style::RegularItalic, fonts::Style::Bold,
      fonts::Style::BoldItalic};
  for (auto family : families) {
    for (auto style : styles) {
      auto selected = font_manager.query(family, style);

      std::cout << "Family: " << family << "_Style:" << style << " -> ";

      if (selected) {
        std::cout << selected.value() << std::endl;
      } else {
        std::cout << "NONE!" << std::endl;
      }
    }
  }
}