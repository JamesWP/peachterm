#include "parser.hpp"

#include <string>
#include <cctype>
#include <cassert>
#include "util.hpp"

#if defined(__unix)
#include <unicode/brkiter.h>
#include <unicode/locid.h>
#include <unicode/utext.h>
#elif defined(WIN32)
#include <icu.h>
#endif

namespace parser {

std::ostream &operator<<(std::ostream &out, STATE s) {
  return (out << static_cast<int>(s));
}

VTParser::VTParser() : state{STATE::NORMAL} {}

void VTParser::parse_input(const char *input, size_t length) {
  UErrorCode status = U_ZERO_ERROR;
  UBreakIterator* iter = ubrk_open(UBRK_CHARACTER, ULOC_UK, nullptr, 0, &status);

  assert(!U_FAILURE(status));

  auto _e = util::ScopeExit([&](){ ubrk_close(iter); });

  UText *text = utext_openUTF8(nullptr, input, length, &status);

  auto _f = util::ScopeExit([&](){ utext_close(text); });

  assert(!U_FAILURE(status));

  ubrk_setUText(iter, text, &status);

  assert(!U_FAILURE(status));

  for(int32_t pos = ubrk_next(iter), last_pos = 0; pos != UBRK_DONE; pos = ubrk_next(iter)) {
    size_t glyph_length = pos - last_pos;

    if (glyph_length == 1u) {
      char c = input[last_pos];
      parse_input(c);
    } else if (glyph_length == 2u &&
               ((input[last_pos] == '\r' && input[last_pos + 1] == '\n') ||
                (input[last_pos] == '\n' && input[last_pos + 1] == '\r'))) {
      // for some reason the character breaker combines return and newline
      on_return();
      on_newline();
    } else {
      on_glyph(input + last_pos, glyph_length);
    }

    last_pos = pos;
  }
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
    case 'M':
      on_ri();
      state = STATE::NORMAL;
      break;
    case '=':
    case '>':
    case '7':
    default:
      dispatch_esc(c);
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

    if (is_final_osi(c)) {
      dispatch_osi(command.data(), command.size());
      state = STATE::NORMAL;
    }

    if (command.size() > 1 && is_final_osi(c, *std::prev(command.end(), 2u))) {
      dispatch_osi(command.data(), command.size());
      state = STATE::NORMAL;
    }

    command.push_back(c);

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
  //std::cout << "OSI: '";
  //std::cout.write(data, length-1);
  //std::cout << "'\n";

  std::string_view operation = {data, length};

  auto pos = operation.find_first_of(";");

  if(pos == std::string_view::npos) {
    // ignore
    return;
  }

  std::string op_num = {operation.data(), operation.data() + pos};

  int op_num_int = std::stoi(op_num);
  operation = {operation.data() + pos + 1, operation.size() - (pos + 1)};

  on_osi(op_num_int, operation);
}

void VTParser::dispatch_csi(const char *data, size_t length) {
  char operation = data[length - 1];

  static std::vector<int> args;
  static std::string options;

  args.clear();
  options.clear();

  for (const char *pos = data; pos < data + length - 1;) {
    if (*pos == '\0')
      break;

    if (std::isdigit(*pos)) {
      char *end;
      long argv = std::strtol(pos, &end, 10);
      pos = end;
      args.push_back(argv);
    } else if (*pos == ';') {
      // ignore seperator.
      pos++;
    } else {
      options.push_back(*pos);
      pos++;
    }
  }

#ifdef PEACHTERM_IS_VERBOSE
  std::cout << "CSI (" << operation << ") ";

  std::cout << "(";
  for (int arg : args) {
    std::cout << arg << ' ';
  }
  std::cout << ") ";

  std::cout << "[";
  std::cout << options;
  std::cout << "]";

  std::cout << " : '";
  std::cout.write(data, length);
  std::cout << "'\n";
#endif

  on_csi(operation, args, options);
}

void VTParser::dispatch_esc(char op) {
  std::cout << "ESC: \"" << op << "\"\n";

  on_esc(op);
}
} // namespace parser
