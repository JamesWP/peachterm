#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

#ifdef __unix
#include <boost/asio.hpp>
#endif

namespace io {
class PseudoTerminal {
public:
  using data_read_cb =
      std::function<void(PseudoTerminal *pt, const char *data, size_t len)>;

private:
  data_read_cb _data_cb; // Data passed to this callback must live untill the matching invocation of `this->read_complete()`

  std::thread t;

#ifdef _WIN32
  void* pseudo_terminal{nullptr};
  void* child_process{nullptr};

  std::mutex read_data_state;
  std::condition_variable read_data_state_cv;
  bool buffer_ready_for_write{true};
  void* child_process_output;
  void* child_process_input;
#endif

#ifdef __unix
  std::vector<char> write_buffer;
  std::vector<char> read_buffer;


  int childfd;
  int parentfd;

  boost::asio::io_service service;
  boost::asio::posix::stream_descriptor stream;
  boost::asio::io_service::work work;
#endif

public:
  PseudoTerminal(data_read_cb);
  ~PseudoTerminal();

  void read_complete();
  // allow next read to happen

  // perform write to child application
  void write(std::string_view data);
  void write(const char *data, size_t len);
  void write(char c);

  bool start();
  // begin communication

  bool set_size(int rows, int cols);

  bool fork_child();
};
} // namespace io
