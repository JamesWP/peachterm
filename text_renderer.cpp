#include "text_renderer.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <iostream>
#include <string_view>
#include <iomanip>

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

  int cache_index = get_cache_location(font, glyph, fg, bg);

  SDL_Rect cached_cell_rect;
  cached_cell_rect.x = (cache_index % cache_width_cells) * cell_width;
  cached_cell_rect.h = (cache_index / cache_height_cells) * cell_height;
  cached_cell_rect.w = cell_rect.w = cell_width;
  cached_cell_rect.h = cell_rect.h = cell_height;

  if (cache_populated(cache_index)) {
    // Cache hit
    SDL_RenderCopy(ren, cacheTex, &cached_cell_rect, &cell_rect);

    return;
  }
  // Cache miss

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
    cell_rect.w = cached_cell_rect.w;
    cell_rect.h = cached_cell_rect.h;
    SDL_RenderCopy(ren, cellTex, &cached_cell_rect, &cell_rect);

    // Restore prvious render target
    SDL_SetRenderTarget(ren, prevTexTarget);
  }

  SDL_DestroyTexture(cellTex);
  SDL_FreeSurface(cellSurf);
}

int TextRenderer::get_cache_location([[maybe_unused]] TTF_Font *font,
                                     [[maybe_unused]] std::string_view glyph,
                                     [[maybe_unused]] SDL_Color fg,
                                     [[maybe_unused]] SDL_Color bg) {
  return 0;
}

bool TextRenderer::cache_populated([[maybe_unused]] int cache_location) {
  return false;
}

void TextRenderer::dump_cache_stats() const {
    std::cout << "Cell cache stats: hits:" << cache_hits << " misses:" << cache_misses << "\n";
    std::cout << "Cell cache stats: efficiency:"  << std::setprecision(2) << static_cast<float>(cache_hits) / (cache_misses + cache_hits) << "\n";
}
} // namespace gfx