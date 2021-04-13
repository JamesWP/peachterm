#include "fonts.hpp"
#include <iostream>
#include <vector>
#include <SDL.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  auto font_manager = fonts::Manager();

  auto families = font_manager.familyList();

  for(auto& f: families) {
    std::cout << f << std::endl;
  }

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

  return 0;
}