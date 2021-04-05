#include "fonts.hpp"
#include <iostream>

#include <Windows.h>

namespace fonts {

std::ostream &operator<<(std::ostream &out, Style s) {
  switch (s) {
  case Style::Regular:
    return (out << "regular");
  case Style::Bold:
    return (out << "bold");
  case Style::RegularItalic:
    return (out << "italic");
  case Style::BoldItalic:
    return (out << "bold italic");
  default:
    assert(false);
    return out;
  }
}

std::vector<std::string> Manager::familyList() {
  std::vector<std::string> family_list;

  return family_list;
}

std::optional<FontDescription> Manager::query(std::optional<std::string> family,
                                              Style style) {
  (void)family;
  (void)style;
  return {};
}
} // namespace fonts