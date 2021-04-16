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
                                              Style style, bool load_data) {
  auto weight = FW_REGULAR;
  if (style == Style::Bold || style == Style::BoldItalic) {
    weight = FW_BOLD;
  }

  auto charset = ANSI_CHARSET;

  auto outprecis = OUT_TT_ONLY_PRECIS;

  auto italic = FALSE;
  if (style == Style::BoldItalic || style == Style::RegularItalic) {
    italic = TRUE;
  }

  auto hfont =
      CreateFontA(0, 0, 0, 0, weight, italic, 0, 0, charset, outprecis, 0, 0, 0,
                  family.has_value() ? family.value().data() : nullptr);

  if (hfont == nullptr) {
    std::cout << "Unable to select font!" << std::endl;
    return {};
  }
  // We found a font!
  // DWORD GetFontData(
  //   HDC   hdc,
  //   DWORD dwTable,
  //   DWORD dwOffset,
  //   PVOID pvBuffer,
  //   DWORD cjBuffer
  // );
  // FontDescription des;
  // des.family = ctx.families_list.front();
  // return des;

  HDC hDC = GetDC(NULL);

  SelectObject(hDC, (HGDIOBJ)hfont);
  //SelectFont(hDC, hfont);

  FontDescription des;
  size_t font_data_size = GetFontData(hDC, 0, 0, nullptr, 0);
  
  if (font_data_size == GDI_ERROR) {
    std::cout << "Unable to size font!" << std::endl;
    return {};
  }
  

  des.family.resize(GetTextFace(hDC, 0, nullptr), '\0');
  GetTextFace(hDC, static_cast<int>(des.family.size()), des.family.data());

  if(load_data) {
    // std::cout << "Size " << font_data_size << std::endl;
    des.font_data.resize(font_data_size, '\0');
    GetFontData(hDC, 0, 0, reinterpret_cast<void*>(des.font_data.data()), static_cast<DWORD>(des.font_data.size()));
  }

  return des;
}
} // namespace fonts

namespace {
int CALLBACK EnumFontFamExProc(const ENUMLOGFONTEX *lpelfe,
                               const TEXTMETRIC *lpntme, DWORD FontType,
                               LPARAM app_pointer) {
  (void)lpntme;

  auto *ctx = (enum_context *)app_pointer;

  auto &families_list = ctx->families_list;

  // Record the number of raster, TrueType, and vector
  // fonts in the font-count array.

  if (ctx->optional_style_filter.has_value()) {
    if (0 != strncmp(reinterpret_cast<const char *>(lpelfe->elfStyle),
                     ctx->optional_style_filter.value().data(),
                     ctx->optional_style_filter.value().size())) {
      // continue enumeration
      return ctx->just_the_one_font_actually ? 0 : 1;
    }
  }

  if (FontType == TRUETYPE_FONTTYPE) {
    auto *fontName = reinterpret_cast<const char *>(lpelfe->elfFullName);
    families_list.push_back(fontName);
  }

  // continue enumeration
  return ctx->just_the_one_font_actually ? 0 : 1;
}
} // namespace