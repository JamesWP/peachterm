#include "io.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace io;
using namespace std::chrono_literals;

void read_data_available(PseudoTerminal *pt, const char *data, size_t len) {
  pt->read_complete();
  printf("Data available: %d bytes at %ld\n", (int)len, (long int)data);
  pt->write(data, len);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  PseudoTerminal pt{read_data_available};

  pt.start();

  std::this_thread::sleep_for(100s);
  return 1;
}
