#include "app.hpp"
#include "graphics.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  std::cout << "Starting" << std::endl;

  gfx::context ctx;

  if (!ctx) {
    std::cerr << "Graphics init failed\n";
  }

  app::run();
}
