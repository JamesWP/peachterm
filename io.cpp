#include "io.hpp"

#include <chrono>
#include <iostream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace io {
PseudoTerminal::PseudoTerminal(data_read_cb data_cb)
    : _data_cb(data_cb), write_buffer(1024, '\0'),
      read_buffer(1024, '\0'), stream{service}, work{service} {}

PseudoTerminal::~PseudoTerminal() {
  service.stop();
  t.join();
}

void PseudoTerminal::read_complete() {
  stream.async_read_some(
      boost::asio::buffer(read_buffer.data(), read_buffer.size()),
      [this](const boost::system::error_code &err, long unsigned int length) {
        if (err) {
          std::cerr << "Error message: " << err.message() << "\n";
          this->_data_cb(this, nullptr, 0u);
          return;
        }

        this->_data_cb(this, read_buffer.data(), length);
      });
}

void PseudoTerminal::write(char data) { write(&data, 1u); }

void PseudoTerminal::write(std::string_view data) {
  write(data.data(), data.size());
}

void PseudoTerminal::write(const char *data, size_t len) {
  std::cout << "Write pt length= " << len << ": ";
  size_t _len = len;
  for (const char *d = data; _len-- > 0; d++) {
    if (std::isprint(*d)) {
      std::cout << *d;
    } else {
      std::cout << '\\' << 'x' << std::hex << (int)(*d & 0xFF) << std::dec;
    }
  }
  std::cout << '\n';

  boost::asio::write(stream, boost::asio::buffer(data, len));
}

bool PseudoTerminal::start() {
  std::cout << "Start PT\n";

  parentfd = posix_openpt(O_RDWR | O_NOCTTY);

  if (parentfd == -1) {
    return false;
  }

  fcntl(parentfd, F_SETFL, fcntl(parentfd, F_GETFL) | O_NONBLOCK);

  if (grantpt(parentfd) == -1) {
    return false;
  }

  if (unlockpt(parentfd) == -1) {
    return false;
  }

  char *pts_name = ptsname(parentfd);

  if (pts_name == nullptr) {
    return false;
  }

  childfd = open(pts_name, O_RDWR | O_NOCTTY);

  if (childfd == -1) {
    return false;
  }

  return true;
}

bool PseudoTerminal::set_size(int rows, int cols) {
  struct winsize ws;
  ws.ws_row = rows;
  ws.ws_col = cols;

  return -1 != ioctl(parentfd, TIOCSWINSZ, &ws);
}

bool PseudoTerminal::fork_child() {

  pid_t p = fork();
  if (p == 0) {
    close(parentfd);

    setsid();

    if (ioctl(childfd, TIOCSCTTY, nullptr) == -1) {
      return false;
    }

    dup2(childfd, 0);
    dup2(childfd, 1);
    dup2(childfd, 2);
    close(childfd);

    ::setenv("TERM", "xterm-256color", 1);

    execlp("/usr/bin/bash", "-/usr/bin/bash", nullptr);
    return false;
  } else if (p > 0) {
    close(childfd);

    stream.assign(parentfd);

    std::thread thread{[this]() {
      std::cout << "IO thread\n";

      this->service.run();

      std::cout << "IO thread exiting\n";
    }};

    std::swap(t, thread);

    read_complete();

    return true;
  }

  return false;
}
} // namespace io
