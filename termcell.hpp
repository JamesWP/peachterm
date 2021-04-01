#pragma once
#include <string>
#include <tuple>
#include <stdint.h>

namespace gfx {
struct TermCell {
  std::string glyph = " ";
  uint32_t fg_col = 0xFFFFFFFF;
  uint32_t bg_col = 0x000000FF;
  bool bold = false;
  bool italic = false;
  bool overline = false;
  bool underline = false;
  bool dunderline = false;
  bool strike = false;
  bool feint = false;
  bool reverse = false;

public:
  enum class Attr {
    BOLD,
    ITALIC,
    FG,
    BG,
    OVERLINE,
    UNDERLINE,
    DUNDERLINE,
    STRIKE,
    FEINT,
    REVERSE
  };

  friend inline bool operator==(const TermCell &l, const TermCell &r) {
    return std::tie(l.fg_col, l.bg_col, l.bold, l.italic, l.overline, 
                    l.underline, l.dunderline, l.strike, l.feint, 
                    l.reverse, l.glyph) ==
           std::tie(r.fg_col, r.bg_col, r.bold, r.italic, l.overline, 
                    r.underline, r.dunderline, r.strike, r.feint, 
                    r.reverse, r.glyph);
  }

  friend inline bool operator!=(const TermCell &l, const TermCell &r) {
    return !(l == r);
  }
};
} // namespace Gfx