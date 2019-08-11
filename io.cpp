#include "io.hpp"

#include <chrono>
#include <iostream>
#include <cassert>

#include <unistd.h>
#include <fcntl.h>

namespace io {
PseudoTerminal::PseudoTerminal(cb _data_available)
    : data_available(_data_available) {}

PseudoTerminal::~PseudoTerminal() {
}

size_t PseudoTerminal::read(char *d, size_t n) {
  (void)d;
  (void)n;
  return 0u;
}

size_t PseudoTerminal::write(const char *d, size_t n) {
  (void)d;
  (void)n;
  return 0u;
}

void PseudoTerminal::start() {
  std::cout << "Start PT\n";
 
  int fds[2]; 
  
  if (0 != pipe2(fds, O_NONBLOCK)) 
  {
    assert(false);
  }

  clientfd = fds[0];
  masterfd = fds[1];

  

}
} // namespace io
