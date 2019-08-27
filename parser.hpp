#pragma once

#include <stddef.h>
#include <iostream>

namespace parser {

enum class STATE { NORMAL, UTF8, ESCAPE, CSI, OSI, CHARSET };

class VTParser {
  std::string command;
  STATE state;

public:
  VTParser();
  virtual ~VTParser() {}

  void parse_input(const char *input, size_t length);
  void parse_input(char c);

  void dispatch_osi(const char *input, size_t length);
  void dispatch_csi(const char *input, size_t length);

public:
  virtual void on_glyph(const char *glyph, size_t length) {
    (void)glyph;
    (void)length;
  }
  virtual void on_newline(){};
  virtual void on_return(){};
  virtual void on_tab(){};
  virtual void on_bell(){};
  virtual void on_backspace(){};
  virtual void on_charset(char c) { (void)c; };

  virtual void on_csi_m(const char *, size_t);
  virtual void on_csi_K(const char *, size_t);
};

} // namespace parser
