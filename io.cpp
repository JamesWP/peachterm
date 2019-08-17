#include "io.hpp"

#include <chrono>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>

namespace io {
PseudoTerminal::PseudoTerminal(data_read_cb data_cb)
    : _data_cb(data_cb), write_buffer(1024, '\0'),
      read_buffer(1024, '\0'), stream{service}, work{service} {}

PseudoTerminal::~PseudoTerminal() { service.stop(); }

void PseudoTerminal::read_complete() {
  stream.async_read_some(
      boost::asio::buffer(read_buffer.data(), read_buffer.size()),
      [this](const boost::system::error_code &err, long unsigned int length) {
        if (err) {
          std::cerr << "Error message: " << err.message() << "\n";
          return;
        }

        this->_data_cb(this, read_buffer.data(), length);
      });
}

void PseudoTerminal::write(const char *data, size_t len) {
  boost::asio::write(
      stream, boost::asio::buffer(data, len));
}

void PseudoTerminal::start() {
  std::cout << "Start PT\n";
 
  int listenfd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un serv_addr;

  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, "socket", sizeof(serv_addr.sun_path));

  unlink("socket");

  bind(listenfd, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr));

  listen(listenfd, 1);

  std::cout << "Waiting to accept connection on socket\n";

  int masterfd = accept(listenfd, nullptr, nullptr);

  stream.assign(masterfd);

  std::thread thread{[this]() {
    std::cout << "IO thread\n";

    this->service.run();

    std::cout << "IO thread exiting\n";
  }};

  t.swap(thread);

  std::cout << "Accepted connection connection on socket\n";

  read_complete();
}
} // namespace io
