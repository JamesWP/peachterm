#include "text_renderer.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <iostream>

namespace {
SDL_RWops *RW_FromString(const std::string &data) {
  return SDL_RWFromConstMem(data.data(), static_cast<int>(data.size()));
}
} // namespace

namespace gfx {

void TextRenderer::load_fonts(const FontSpec &spec) {
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
}

TextRenderer::~TextRenderer() {
    std::cout << "Destroying fonts\n";

    if(fontRegular) {
        TTF_CloseFont(fontRegular);
    }
    if(fontBold) {
        TTF_CloseFont(fontBold);
    }
    if(fontRegularItalic) {
        TTF_CloseFont(fontRegularItalic);
    }
    if(fontBoldItalic) {
        TTF_CloseFont(fontBoldItalic);
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
void TextRenderer::dump_cache_stats() const {
    std::cout << "Cell cache stats: hits:" << cache_hits << " misses:" << cache_misses << "\n";
    std::cout << "Cell cache stats: efficiency:"  << std::setprecision(2) << static_cast<float>(cache_hits) / (cache_misses + cache_hits) << "\n";
}
} // namespace gfx