#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include <boost/asio.hpp>

namespace io {
class PseudoTerminal {
public:
  using cb = std::function<void(void)>;

private:
  cb data_available;
  
  int clientfd;
  int masterfd; 

  std::vector<char> write_buffer;
  std::vector<char> read_buffer;

  void run();

public:
  PseudoTerminal(cb);
  ~PseudoTerminal();

  size_t read(char *d, size_t n);
  size_t write(const char *d, size_t);

  void start();
};
}
