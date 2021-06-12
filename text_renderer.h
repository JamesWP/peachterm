#pragma once
#include <string>

struct _TTF_Font;
typedef _TTF_Font TTF_Font;

namespace gfx {

class FontSpec {
public:
  std::string regular;
  std::string bold;
  std::string italic;
  std::string bolditalic;
  int pointsize;
};

class TextRenderer {
  TTF_Font *fontRegular = nullptr;
  TTF_Font *fontRegularItalic = nullptr;
  TTF_Font *fontBold = nullptr;
  TTF_Font *fontBoldItalic = nullptr;

public:
  int cell_width = 6;
  int cell_height = 12;
  int font_height = 12;
  int font_point = 14;

  void load_fonts(const FontSpec &);
  TTF_Font *get_font(bool bold = false, bool italic = false) const;
  std::pair<int, int> cell_size() const;
  ~TextRenderer();
};
} // namespace gfx