#pragma once

#include <iostream>
#include <stddef.h>
#include <string_view>
#include <vector>

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
  void dispatch_esc(char op);

public:
  virtual void on_glyph(const char *glyph, size_t length) {
    (void)glyph;
    (void)length;
  }
  virtual void on_newline(){}
  virtual void on_return(){}
  virtual void on_tab(){}
  virtual void on_backspace(){}
  virtual void on_bell(){}
  virtual void on_charset(char c) { (void)c; }
  virtual void on_csi(char op, const std::vector<int> &args,
                      std::string_view options) {
    (void)op;
    (void)args;
    (void)options;
  }
  virtual void on_ri(){}
  virtual void on_esc(char op){ (void)op; }
};

} // namespace parser
