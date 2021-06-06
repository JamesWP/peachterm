#include "fonts.hpp"
#include <iostream>
#include <fstream>

#include <fontconfig/fontconfig.h>

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
  FcPattern *pat = FcPatternCreate();
  FcPatternAddInteger(pat, FC_SPACING, 100); // Query for monospace fonts
  FcPatternAddString(pat, FC_FONTFORMAT, (FcChar8*)"TrueType"); // Query for ttf fonts
  auto _pat = util::ScopeExit([&]() { FcPatternDestroy(pat); });
  FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, (char *)0);
  auto _os = util::ScopeExit([&]() { FcObjectSetDestroy(os); });
  FcFontSet *fs = FcFontList(0, pat, os);
  auto _fs = util::ScopeExit([&]() { FcFontSetDestroy(fs); });

  if (!fs) {
    std::cerr << "FontList failed" << std::endl;
    return {};
  }

  std::vector<std::string> family_list;
  family_list.reserve(fs->nfont);

  for (int i = 0; i < fs->nfont; i++) {
    FcPattern *font = fs->fonts[i];
    const char *familyStr = nullptr;
    if (FcResultMatch == FcPatternGetString(font, FC_FAMILY,
                                            0 /* zeroth value for object*/,
                                            (FcChar8 **)&familyStr)) {
      family_list.emplace_back(familyStr);
    } else {
      std::cerr << "FontList font has no family" << std::endl;
      return {};
    }
  }

  return family_list;
}

std::optional<FontDescription> Manager::query(std::optional<std::string> family,
                                              Style style, bool load_data) {
  FcPattern *pat = FcPatternCreate();
  auto _pat = util::ScopeExit([&]() { FcPatternDestroy(pat); });

  FcPatternAddInteger(pat, FC_SPACING, 100); // Query for monospace fonts
  FcPatternAddString(pat, FC_FONTFORMAT, (FcChar8*)"TrueType"); // Query for ttf fonts
  if(family){
    FcPatternAddString(pat, FC_FAMILY, (FcChar8*)family.value().c_str()); // Query for specified family fonts
  }

  std::ostringstream styleStr;
  styleStr << style;
  FcPatternAddString(pat, FC_STYLE, (FcChar8*) styleStr.str().c_str()); // Query for this style fonts
  
  FcConfigSubstitute (0, pat, FcMatchPattern); // unsure ??? fc-match does this
  FcDefaultSubstitute (pat); // unsure ??? fc-match does this

  FcObjectSet *os = FcObjectSetBuild(FC_FULLNAME, FC_FILE, (char *)0);
  auto _os = util::ScopeExit([&]() { FcObjectSetDestroy(os); });
  FcFontSet *fs = FcFontList(0, pat, os);
  auto _fs = util::ScopeExit([&]() { FcFontSetDestroy(fs); });

  if (!fs) {
    std::cerr << "FontList failed" << std::endl;
    return {};
  }

  FcResult result;
  auto match = FcFontMatch(0, pat, &result);

  if(!match) {
    return {};
  }

  const char *fullName = nullptr;
  if (FcResultMatch != FcPatternGetString(match, FC_FULLNAME,
                                          0 /* zeroth value for object*/,
                                          (FcChar8 **)&fullName)) {
    return {};
  }

  const char *file = nullptr;
  if (FcResultMatch != FcPatternGetString(match, FC_FILE,
                                          0 /* zeroth value for object*/,
                                          (FcChar8 **)&file)) {
    return {};
  }
  
  FontDescription des;
  des.family = fullName;

  if(load_data) {
    std::ifstream font_file;
    // open at eof to get size with tellg
    font_file.open(file, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);

    if(!font_file) {
      std::cerr << "File: " << file << std::endl;
      std::cerr << "Unable to open font file" << std::endl;
      return {};
    }

    auto font_size = font_file.tellg();

    // rewind to begining
    font_file.seekg(0);

    auto begin = std::istreambuf_iterator<char>(font_file);
    auto end = std::istreambuf_iterator<char>{};

    des.font_data.reserve(font_size);
    des.font_data.assign(begin, end);
  }

  return des;
}
} // namespace fonts
