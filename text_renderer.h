#pragma once
#include <string>
#include <string_view>

struct _TTF_Font;
struct SDL_Renderer;
struct SDL_Color;
struct SDL_Texture;
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

  // Cell cache
  static constexpr int cache_width_cells = 16;  // cache size in cells
  static constexpr int cache_height_cells = 16; // cache size in cells
  SDL_Texture *cacheTex = nullptr;

  int cache_hits = 0;
  int cache_misses = 0;

public:
  int cell_width = 6;
  int cell_height = 12;
  int font_height = 12;
  int font_point = 14;

  void load_fonts(SDL_Renderer *ren, const FontSpec &);
  TTF_Font *get_font(bool bold = false, bool italic = false) const;
  std::pair<int, int> cell_size() const;
  void draw_character(SDL_Renderer *ren, TTF_Font *font, std::string_view glyph,
                      const SDL_Color &fg, const SDL_Color &bg, int top,
                      int left);
  void dump_cache_stats() const;
  ~TextRenderer();

private:
  int get_cache_location(TTF_Font *font, std::string_view glyph, SDL_Color fg,
                         SDL_Color bg);
  bool cache_populated(int cache_location);
};
} // namespace gfx