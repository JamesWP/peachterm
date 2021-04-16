#pragma once
#include "util.hpp"
#include <cassert>
#include <iomanip>
#include <optional>
#include <vector>
#include <variant>

namespace fonts {
class FontDescription {
public:
  std::string family;
  std::string font_data;

  friend std::ostream &operator<<(std::ostream &out,
                                  const FontDescription &des) {
    return (out << "Font(" << std::quoted(des.family)
                << ", size:" << des.font_data.size() << "(bytes))");
  }
};

enum class Style { Regular, Bold, RegularItalic, BoldItalic };

std::ostream &operator<<(std::ostream &out, Style s);

class Manager {
public:
  std::optional<FontDescription> defaultFont() {
    return query({}, Style::Regular, true);
  }

  std::vector<std::string> familyList();

  std::optional<FontDescription> query(std::optional<std::string> family,
                                       Style, bool load_data);
};

} // namespace fonts
