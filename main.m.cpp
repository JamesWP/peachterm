#include "graphics.hpp"
#include "app.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  gfx::context ctx;

  if (!ctx) {
    std::cerr << "Graphics init failed\n";
  }
  
  app::run();
}

