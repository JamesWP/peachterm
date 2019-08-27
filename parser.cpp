#include "parser.hpp"

#include <unicode/brkiter.h>
#include <unicode/utext.h>
#include <unicode/locid.h>

namespace parser {

std::ostream &operator<<(std::ostream &out, STATE s) {
  return (out << static_cast<int>(s));
}

VTParser::VTParser() : state{STATE::NORMAL} {}

void VTParser::parse_input(const char *input, size_t length) {
  UErrorCode err = U_ZERO_ERROR;

  UText *text = utext_openUTF8(nullptr, input, length, &err);

  if (U_FAILURE(err))
    throw std::runtime_error("Failed to open text");

  icu::Locale locale;

  icu::BreakIterator *it =
      icu::BreakIterator::createCharacterInstance(locale, err);

  if (U_FAILURE(err))
    throw std::runtime_error("Failed to create break it");

  it->setText(text, err);

  if (U_FAILURE(err))
    throw std::runtime_error("Failed to create break it");

  uint32_t end = static_cast<uint32_t>(icu::BreakIterator::DONE);

  uint32_t last_pos = it->first();

  for (uint32_t pos = it->next(); pos != end; pos = it->next()) {
    size_t length = pos - last_pos;
    if (length == 1u) {
      char c = input[last_pos];
      parse_input(c);
    } else if (length == 2u &&
               ((input[last_pos] == '\r' && input[last_pos + 1] == '\n') ||
                (input[last_pos] == '\n' && input[last_pos + 1] == '\r'))) {
      // for some reason the character breaker combines return and newline
      on_return();
      on_newline();
    } else {
      on_glyph(input + last_pos, length);
    }
    last_pos = pos;
  }

  delete it;
  utext_close(text);
}

bool is_final_csi(char c) { return c >= 0x40 && c <= 0x7c; }
bool is_final_osi(char c) { return c == 7; }
bool is_final_osi(char c, char b) { return c == '\\' && b == '\33'; }

void VTParser::parse_input(char c) {
  switch (state) {
  case STATE::NORMAL:
    // clang-format off
    switch(c) {
      case '\33': state = STATE::ESCAPE;  break;

      case '\n': on_newline();            break;
      case '\r': on_return();             break;
      case '\t': on_tab();                break;
      case '\a': on_bell();               break;
      case '\b': on_backspace();          break;
      case '\0': /* ignore */             break;

      default:   on_glyph(&c, 1u);        break;
      // clang-format on
    }
    break;
  case STATE::ESCAPE:
    switch (c) {
    case '[':
      state = STATE::CSI;
      command.clear();
      break;
    case ']':
      state = STATE::OSI;
      command.clear();
      break;
    case '(':
      state = STATE::CHARSET;
      break;
    case '=':
    case '>':
    case '7':
    default:
      state = STATE::NORMAL;
    }
    break;
  case STATE::CSI:
    command.push_back(c);
    if (is_final_csi(c)) {
      dispatch_csi(command.data(), command.size());
      state = STATE::NORMAL;
    }
    break;
  case STATE::OSI:
    command.push_back(c);

    if (is_final_osi(c)) {
      dispatch_osi(command.data(), command.size());
      state = STATE::NORMAL;
    }

    if (command.size() > 1 && is_final_osi(c, *std::prev(command.end(), 2u))) {
      dispatch_osi(command.data(), command.size());
      state = STATE::NORMAL;
    }

    break;
  case STATE::CHARSET:
    state = STATE::NORMAL;
    on_charset(c);
    break;
  default:
    std::cout << "Unknown state " << state << "\n";
    state = STATE::NORMAL;
    break;
  }
}

void VTParser::dispatch_osi(const char *data, size_t length) {
  std::cout << "OSI: '";
  std::cout.write(data, length);
  std::cout << "'\n";
}

void VTParser::dispatch_csi(const char *data, size_t length) {
  char operation = data[length - 1];

  std::cout << "CSI (" << operation << ") : '";
  std::cout.write(data, length);
  std::cout << "'\n";

  switch (operation) {
  case 'm':
    return on_csi_m(data, length);
  case 'K':
    return on_csi_K(data, length);
  default:
    return;
  }
}

void VTParser::on_csi_m(const char *, size_t) {}
void VTParser::on_csi_K(const char *, size_t) {}

} // namespace parser
