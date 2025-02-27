#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>

namespace gfx {

class FontSpec {
public:
  std::string regular;
  std::string bold;
  std::string italic;
  std::string bolditalic;
  int pointsize;
};

using CellCacheKey = std::tuple<TTF_Font *, SDL_Color, SDL_Color, std::string>;

struct cell_cache_key_hash {
  size_t operator()(CellCacheKey const &p) const;
};

using CacheList = std::list<std::pair<CellCacheKey, int>>;
using CacheMap =
    std::unordered_map<CellCacheKey, CacheList::iterator, cell_cache_key_hash>;

class TextRenderer {
  TTF_Font *fontRegular = nullptr;
  TTF_Font *fontRegularItalic = nullptr;
  TTF_Font *fontBold = nullptr;
  TTF_Font *fontBoldItalic = nullptr;

  // Cell cache
  static constexpr int cache_width_cells = 48;  // cache size in cells
  static constexpr int cache_height_cells = 48; // cache size in cells
  SDL_Texture *cacheTex = nullptr;

  int cache_hits = 0;
  int cache_misses = 0;

  CacheList lru_list;
  CacheMap lru_map;

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
  void dump_cache_stats();
  void dump_cache_to_disk(SDL_Renderer *ren) const;
  ~TextRenderer();

private:
  // returns the (possibley new) cache location of the item.
  // {index_location, is_empty}
  std::pair<int, bool> get_cache_location(TTF_Font *font,
                                          std::string_view glyph, SDL_Color fg,
                                          SDL_Color bg);
};
} // namespace gfx