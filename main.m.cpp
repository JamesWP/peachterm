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


#ifdef __unix
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
#endif

#ifdef _WIN32
  std::string family = "Courier New";

  gfx::FontSpec spec;
  spec.regular = family + "-R.ttf";
  spec.bold = family + "-B.ttf";
  spec.bolditalic = family + "-BI.ttf";
  spec.italic = family + "-RI.ttf";
  spec.pointsize = 14;
#endif

  app::run(spec);
}
