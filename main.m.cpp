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

  std::string family = "UbuntuMono";
  gfx::FontSpec spec;
  fonts::Manager font_manager;

  auto regular = font_manager.query(family, fonts::Style::Regular);
  auto bold = font_manager.query(family, fonts::Style::Bold);
  auto bolditalic = font_manager.query(family, fonts::Style::BoldItalic);
  auto italic = font_manager.query(family, fonts::Style::RegularItalic);

  if (!regular || !bold || !bolditalic || !italic) {
    std::cerr << "Unable to load all fonts" << std::endl;
    return 1;
  }

  spec.regular = regular.value().path;
  spec.bold = bold.value().path;
  spec.bolditalic = bolditalic.value().path;
  spec.italic = italic.value().path;
  spec.pointsize = 24;

  app::run(spec);
}
