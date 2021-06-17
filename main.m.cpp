#include "app.hpp"
#include "fonts.hpp"
#include "graphics.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  gfx::context ctx;

  if (!ctx) {
    std::cerr << "Graphics init failed\n";
  }

  std::string family = "Ubuntu Mono";

  gfx::FontSpec spec;
  fonts::Manager font_manager;

  auto regular = font_manager.query(family, fonts::Style::Regular, true);
  auto bold = font_manager.query(family, fonts::Style::Bold, true);
  auto bolditalic = font_manager.query(family, fonts::Style::BoldItalic, true);
  auto italic = font_manager.query(family, fonts::Style::RegularItalic, true);

  if (!regular || !bold || !bolditalic || !italic) {
    std::cerr << "Unable to load all fonts" << std::endl;
    return 1;
  }

  spec.regular = regular.value().font_data;
  spec.bold = bold.value().font_data;
  spec.bolditalic = bolditalic.value().font_data;
  spec.italic = italic.value().font_data;
  spec.pointsize = 16;

  app::run(spec);

  return 0;
}
