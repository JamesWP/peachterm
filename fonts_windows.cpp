#include "fonts.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include <SDL.h>
#include <SDL_syswm.h>
#include <Windows.h>
#include <dwrite_3.h>

namespace {
int CALLBACK EnumFontFamExProc(const ENUMLOGFONTEX *lpelfe,
                               const TEXTMETRIC *lpntme, DWORD FontType,
                               LPARAM lParam);

struct enum_context {
  std::optional<std::string> optional_style_filter;
  std::vector<std::string> families_list;
  bool just_the_one_font_actually = false;
};
} // namespace

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
  enum_context ctx;

  ctx.optional_style_filter = std::string("Regular");

  LOGFONT lf;
  memset(&lf, 0, sizeof(LOGFONT));

  lf.lfFaceName[0] = '\0';
  lf.lfCharSet = ANSI_CHARSET;

  HDC hDC = GetDC(NULL);

  EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc,
                     reinterpret_cast<LPARAM>(&ctx), 0);

  return ctx.families_list;
}

std::optional<FontDescription> Manager::query(std::optional<std::string> family,
                                              Style style) {
  (void)family;
  (void)style;

  assert(family.has_value());
  enum_context ctx;

  ctx.just_the_one_font_actually = true;

  switch (style) {
  case Style::Regular:
    ctx.optional_style_filter = std::string("Regular");
    break;
  case Style::Bold:
    ctx.optional_style_filter = std::string("Bold");
    break;
  case Style::BoldItalic:
    ctx.optional_style_filter = std::string("Bold Italic");
    break;
  case Style::RegularItalic:
    ctx.optional_style_filter = std::string("Italic");
    break;
  };

  LOGFONT lf;
  memset(&lf, 0, sizeof(LOGFONT));

  strncpy(lf.lfFaceName, family.value().data(),
          std::min(family.value().size(), (size_t)LF_FACESIZE));
  lf.lfCharSet = ANSI_CHARSET;

  HDC hDC = GetDC(NULL);

  EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc,
                     reinterpret_cast<LPARAM>(&ctx), 0);

  if (ctx.families_list.size() == 0) {
    if(style != Style::Regular) {
      return query(family, Style::Regular);
    } else {
      return {};
    }
  }

  // We found a font!
  // DWORD GetFontData(
  //   HDC   hdc,
  //   DWORD dwTable,
  //   DWORD dwOffset,
  //   PVOID pvBuffer,
  //   DWORD cjBuffer
  // );
  FontDescription des;
  des.family = ctx.families_list.front();
  return des;
}
} // namespace fonts

namespace {
int CALLBACK EnumFontFamExProc(const ENUMLOGFONTEX *lpelfe,
                               const TEXTMETRIC *lpntme, DWORD FontType,
                               LPARAM app_pointer) {
  (void)lpntme;

  auto *ctx = (enum_context *)app_pointer;
  assert(families_list_p);

  auto &families_list = ctx->families_list;

  // Record the number of raster, TrueType, and vector
  // fonts in the font-count array.

  if (ctx->optional_style_filter.has_value()) {
    if (0 != strncmp(reinterpret_cast<const char *>(lpelfe->elfStyle),
                     ctx->optional_style_filter.value().data(),
                     ctx->optional_style_filter.value().size())) {
      // continue enumeration
      return ctx->just_the_one_font_actually?0:1;
    }
  }

  if (FontType == TRUETYPE_FONTTYPE) {
    auto *fontName = reinterpret_cast<const char *>(lpelfe->elfFullName);
    families_list.push_back(fontName);
  }

  // continue enumeration
  return ctx->just_the_one_font_actually?0:1;
}
} // namespace