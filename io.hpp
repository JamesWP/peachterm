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
  using data_read_cb =
      std::function<void(PseudoTerminal *pt, 
                         const char *data, 
                         size_t len)>;

private:
  data_read_cb _data_cb;

  int childfd;
  int parentfd; 

  std::vector<char> write_buffer;
  std::vector<char> read_buffer;

  boost::asio::io_service service;
  boost::asio::posix::stream_descriptor stream;

  std::thread t;

  boost::asio::io_service::work work;

public:
  PseudoTerminal(data_read_cb);
  ~PseudoTerminal();

  void read_complete();
  // allow next read to happen

  // perform write to child application
  void write(const char *data, size_t len);
  void write(char c);

  bool start();
  // begin communication

  bool set_size(int rows, int cols); 

  bool fork_child();
};
}
