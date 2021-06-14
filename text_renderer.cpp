#include "text_renderer.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>

//#define DEBUG_CACHE

bool operator==(const SDL_Color &a, const SDL_Color &b) {
  auto &&t = [](auto &&a) { return std::tie(a.a, a.r, a.g, a.b); };
  return t(a) == t(b);
}

namespace {

SDL_RWops *RW_FromString(const std::string &data) {
  return SDL_RWFromConstMem(data.data(), static_cast<int>(data.size()));
}
} // namespace

namespace gfx {

void TextRenderer::load_fonts(SDL_Renderer *ren, const FontSpec &spec) {
  if (fontRegular != 0) {
    TTF_CloseFont(fontRegular);
  }
  fontRegular = TTF_OpenFontRW(RW_FromString(spec.regular), 1, spec.pointsize);
  assert(fontRegular);

  if (fontRegularItalic != 0) {
    TTF_CloseFont(fontRegularItalic);
  }
  fontRegularItalic =
      TTF_OpenFontRW(RW_FromString(spec.italic), 1, spec.pointsize);
  assert(fontRegularItalic);

  if (fontBold != 0) {
    TTF_CloseFont(fontBold);
  }
  fontBold = TTF_OpenFontRW(RW_FromString(spec.bold), 1, spec.pointsize);
  assert(fontBold);

  if (fontBoldItalic != 0) {
    TTF_CloseFont(fontBoldItalic);
  }
  fontBoldItalic =
      TTF_OpenFontRW(RW_FromString(spec.bolditalic), 1, spec.pointsize);
  assert(fontBoldItalic);

  int advance;
  TTF_GlyphMetrics(fontRegular, '&', nullptr, nullptr, nullptr, nullptr,
                   &advance);

  font_height = TTF_FontHeight(fontRegular);

  cell_height = font_height;
  cell_width = advance;
  font_point = spec.pointsize;

  std::cout << "Loaded fonts\n";

  if (cacheTex) {
    SDL_DestroyTexture(cacheTex);
  }

  cacheTex = SDL_CreateTexture(
      ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      cache_width_cells * cell_width, cache_height_cells * cell_height);

  lru_map.clear();
  lru_list.clear();

  SDL_Color black;
  black.r = black.g = black.b = 0;
  black.a = 255;
  for (int i = 0; i < cache_height_cells * cache_width_cells; i++) {
    auto key =
        std::make_tuple((TTF_Font *)nullptr, black, black, std::string{" "});
    lru_list.push_front(std::make_pair(key, i));
  }

  std::cout << "Cleared cell cache\n";
}

TextRenderer::~TextRenderer() {
  std::cout << "Destroying fonts\n";

  if (fontRegular) {
    TTF_CloseFont(fontRegular);
  }
  if (fontBold) {
    TTF_CloseFont(fontBold);
  }
  if (fontRegularItalic) {
    TTF_CloseFont(fontRegularItalic);
  }
  if (fontBoldItalic) {
    TTF_CloseFont(fontBoldItalic);
  }

  std::cout << "Destroying cell cache\n";

  if (cacheTex) {
    SDL_DestroyTexture(cacheTex);
  }
}

TTF_Font *TextRenderer::get_font(bool bold, bool italic) const {
  // Cell font;
  TTF_Font *font = nullptr;
  if (!bold && !italic)
    font = fontRegular;
  if (!bold && italic)
    font = fontRegularItalic;
  if (bold && !italic)
    font = fontBold;
  if (bold && italic)
    font = fontBoldItalic;

  assert(font);
  return font;
}

std::pair<int, int> TextRenderer::cell_size() const {
  return {cell_width, cell_height};
}

void TextRenderer::draw_character(SDL_Renderer *ren, TTF_Font *font,
                                  std::string_view glyph, const SDL_Color &fg,
                                  const SDL_Color &bg, int top, int left) {
  // Rectangle for the cell. in screen space.
  SDL_Rect cell_rect;
  cell_rect.x = left;
  cell_rect.y = top;
  // width and height as yet unknown

  auto [cache_index, is_hit] = get_cache_location(font, glyph, fg, bg);

  SDL_Rect cached_cell_rect;
  cached_cell_rect.x = (cache_index % cache_width_cells) * cell_width;
  cached_cell_rect.y = (cache_index / cache_height_cells) * cell_height;
  cached_cell_rect.w = cell_rect.w = cell_width;
  cached_cell_rect.h = cell_rect.h = cell_height;

  if (is_hit) {
    SDL_RenderCopy(ren, cacheTex, &cached_cell_rect, &cell_rect);
    return;
  }

  // Begin draw character.
  SDL_Surface *cellSurf = TTF_RenderUTF8_Shaded(font, glyph.data(), fg, bg);
  SDL_Texture *cellTex = SDL_CreateTextureFromSurface(ren, cellSurf);

  // Read back the size of the character
  SDL_QueryTexture(cellTex, nullptr, nullptr, &cell_rect.w, &cell_rect.h);

  SDL_RenderCopy(ren, cellTex, NULL, &cell_rect);

  // Caching
  if (cacheTex != nullptr) {
    // Rememebr previous render target
    SDL_Texture *prevTexTarget = SDL_GetRenderTarget(ren);

    // Copy to cache
    SDL_SetRenderTarget(ren, cacheTex);

    // newly rendered glyph might be slightly different size.
    // use the size from the actual rendered glyph.
    // use the position from the cache destination
    cached_cell_rect.w = cell_rect.w;
    cached_cell_rect.h = cell_rect.h;

    cell_rect.x = 0;
    cell_rect.y = 0;

    SDL_RenderCopy(ren, cellTex, &cell_rect, &cached_cell_rect);
    SDL_RenderFlush(ren);

    // Restore prvious render target
    SDL_SetRenderTarget(ren, prevTexTarget);
  }

  SDL_FreeSurface(cellSurf);
  SDL_DestroyTexture(cellTex);
}

std::pair<int, bool> TextRenderer::get_cache_location(TTF_Font *font, std::string_view glyph,
                                     SDL_Color fg, SDL_Color bg) {
  auto map_key = std::make_tuple(font, fg, bg, std::string(glyph));
  auto map_it = lru_map.find(map_key);
  if (map_it != lru_map.end()) {

    // Cache hit
    ++cache_hits;

    auto list_it = map_it->second;
    auto list_item = *list_it;

#ifdef DEBUG_CACHE
    if (!glyph.empty() && !std::iswspace(glyph.front())) {
      std::cout << "CACHE: Hit " << std::quoted(glyph) << " @ "
                << list_item.second << std::endl;
    }
#endif

    // mark entry as recently used
    lru_list.erase(list_it);
    lru_list.push_front(list_item);
    lru_map.insert(std::make_pair(map_key, lru_list.begin()));

    return {list_item.second, true};
  }

  ++cache_misses;

  // Evict oldest member of cache
  auto oldest_index = lru_list.back().second;
  auto oldest_key = lru_list.back().first;
  lru_list.pop_back();
  lru_map.erase(oldest_key);

#ifdef DEBUG_CACHE
  std::cout << "CACHE: Miss " << std::quoted(glyph) << " + " << oldest_index
            << std::endl;
#endif

  // Emplace new item in cache using newly freed index
  lru_list.push_front(std::make_pair(map_key, oldest_index));
  lru_map.insert(std::make_pair(map_key, lru_list.begin()));

  return {oldest_index, false};
}

void TextRenderer::dump_cache_stats() {
  std::cout << "Cell cache stats: hits:" << cache_hits
            << " misses:" << cache_misses << "\n";
  std::cout << "Cell cache stats: efficiency:" << std::setprecision(2)
            << static_cast<float>(cache_hits) / (cache_misses + cache_hits)
            << "\n";

  cache_hits = cache_misses = 0;
}

size_t cell_cache_key_hash::operator()(const CellCacheKey &p) const {
  auto pointer_hash = std::hash<void *>{}(std::get<0>(p));

  auto color1_hash = std::hash<uint32_t>{}(
      *reinterpret_cast<const uint32_t *>(&std::get<1>(p)));
  auto color2_hash = std::hash<uint32_t>{}(
      *reinterpret_cast<const uint32_t *>(&std::get<2>(p)));

  auto string_hash = std::hash<std::string>{}(std::get<3>(p));

  return string_hash ^ pointer_hash ^ color1_hash ^ color2_hash;
}

void save_texture(std::string_view file_name, SDL_Renderer *renderer,
                  SDL_Texture *texture) {
  SDL_Texture *target = SDL_GetRenderTarget(renderer);
  SDL_SetRenderTarget(renderer, texture);
  int width, height;
  SDL_QueryTexture(texture, NULL, NULL, &width, &height);
  SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
  SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels,
                       surface->pitch);
  SDL_SaveBMP(surface, file_name.data());
  SDL_FreeSurface(surface);
  SDL_SetRenderTarget(renderer, target);
}

void TextRenderer::dump_cache_to_disk(SDL_Renderer *ren) const {
  static int idx = 0;
  idx++;

  save_texture("cell_cache_" + std::to_string(idx), ren, cacheTex);
}

} // namespace gfx