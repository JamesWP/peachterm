#pragma once
#include "util.hpp"
#include <cassert>
#include <iomanip>
#include <optional>
#include <vector>

namespace fonts {
class FontDescription {
public:
  std::string family;
  std::string path;

  friend std::ostream &operator<<(std::ostream &out,
                                  const FontDescription &des) {
    return (out << "Font(" << std::quoted(des.family)
                << ", path:" << std::quoted(des.path) << ")");
  }
};

enum class Style { Regular, Bold, RegularItalic, BoldItalic };

std::ostream &operator<<(std::ostream &out, Style s);

class Manager {
public:
  std::optional<FontDescription> defaultFont() {
    return query({}, Style::Regular);
  }

  std::vector<std::string> familyList();

  std::optional<FontDescription> query(std::optional<std::string> family,
                                       Style);
};

} // namespace fonts
