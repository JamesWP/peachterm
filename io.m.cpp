#include "io.hpp"

#include <iostream>
#include <chrono>
#include <thread>

using namespace io;
using namespace std::chrono_literals;

void data_available() { 
  std::cout << "Data available\n"; 
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  PseudoTerminal pt{data_available};

  pt.start();

  std::this_thread::sleep_for(10s);
  return 1;
}
