#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace io {
class PseudoTerminal {
public:
  using cb = std::function<void(void)>;

private:
  cb data_available;

  std::thread t;

  bool stop{false};

  std::mutex stop_mutex;

  std::condition_variable stop_signal;

  void run();

public:
  PseudoTerminal(cb);
  ~PseudoTerminal();

  size_t read(char *d, size_t n);
  size_t write(const char *d, size_t);

  void start();
};
}
