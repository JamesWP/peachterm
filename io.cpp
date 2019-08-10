#include "io.hpp"

#include <chrono>
#include <iostream>

namespace io {
PseudoTerminal::PseudoTerminal(cb _data_available)
    : data_available(_data_available) {}

PseudoTerminal::~PseudoTerminal() {
  std::unique_lock<std::mutex> g{stop_mutex};
  stop = true;
  stop_signal.notify_one();
  g.unlock();
  t.join();
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

void PseudoTerminal::run() {
  std::cout << "Starting PT\n";

  std::unique_lock<std::mutex> g{stop_mutex};

  stop_signal.wait(g, [this]() { return this->stop; });

  std::cout << "Stopping PT\n";
}

void PseudoTerminal::start() {
  std::cout << "Start PT\n";

  std::thread thread{[this]() { this->run(); }};

  t.swap(thread);
}
} // namespace io
