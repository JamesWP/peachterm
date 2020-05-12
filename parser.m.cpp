#include "parser.hpp"
#include "sample_vim.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace parser;
using namespace testing;

class MockVTParser : public parser::VTParser {
public:
  MOCK_METHOD2(on_glyph, void(const char *, size_t));
  MOCK_METHOD0(on_newline, void(void));
  MOCK_METHOD0(on_return, void(void));
  MOCK_METHOD2(on_csi, void(const char *, size_t));
  MOCK_METHOD2(on_osi, void(const char *, size_t));
};

TEST(VTParser, Simple) {
  MockVTParser p;

  EXPECT_CALL(p, on_newline());
  EXPECT_CALL(p, on_glyph(_, 1u)).Times(18).WillRepeatedly(Return());

  char input[] = "this is some input\n";
  p.parse_input(input, sizeof(input) - 1);
}

TEST(VTParser, ControlSeq) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 1u)).Times(15).WillRepeatedly(Return());

  char input[] = "\33[38;5;121msize_t\33[m length);";
  p.parse_input(input, sizeof(input) - 1);
}

TEST(VTParser, UTF8) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 2u));

  char input[] = "\xc3\xb7";
  p.parse_input(input, sizeof(input) - 1);
}

TEST(VTParser, Emoji) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 4u));

  char input[] = "\xF0\x9F\x98\x82";
  p.parse_input(input, sizeof(input) - 1);
}

TEST(VTParser, BashPrompt) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 1u)).WillRepeatedly(Return());

  char input[] =
      "\033]0;jpeach6@J-MacBookAir:~/gits/term/jterm\033\\"
      "\033]7;file://J-MacBookAir/home/jpeach6/gits/term/jterm\033\\";

  p.parse_input(input, sizeof(input) - 1);
}

TEST(VTParser, Newline) {
  MockVTParser p;

  EXPECT_CALL(p, on_newline());
  EXPECT_CALL(p, on_return());

  char input[] = "\r\n";

  p.parse_input(input, sizeof(input) - 1);
}

TEST(VTParser, LargeSampleVim) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, _)).WillRepeatedly(Return());

  p.parse_input(sample_vim, sizeof(sample_vim));
}

TEST(VTParser, MinimalMan) {
  MockVTParser p;

  unsigned char input[] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65,
  0x20, 0x20, 0x61, 0x72, 0x65, 0x20, 0x61, 0x20, 0x6c, 0x6f, 0x74, 0x20,
  0x6f, 0x66, 0x20, 0x65, 0x6e, 0x68, 0x61, 0x6e, 0x63, 0x65, 0x6d, 0x65,
  0x6e, 0x74, 0x73, 0x20, 0x61, 0x62, 0x6f, 0x76, 0x65, 0x20, 0x56, 0x69,
  0x3a, 0x20, 0x6d, 0x75, 0x6c, 0x74, 0x69, 0x20, 0x6c, 0x65, 0x76, 0x65,
  0x6c, 0x20, 0x75, 0x6e, 0x64, 0x6f, 0x2c, 0x20, 0x6d, 0x75, 0x6c, 0x74,
  0x69, 0x20, 0x77, 0x69, 0x6e, 0xe2, 0x80, 0x90, 0x1b, 0x5b, 0x6d, 0x0d,
  0x0a, 0x1b, 0x5b, 0x37, 0x6d, 0x20, 0x4d, 0x61, 0x6e, 0x75, 0x61, 0x6c,
  0x20, 0x70, 0x61, 0x67, 0x65, 0x20, 0x76, 0x69, 0x6d, 0x28, 0x31, 0x29,
  0x20, 0x6c, 0x69, 0x6e, 0x65, 0x20, 0x31, 0x20, 0x28, 0x70, 0x72, 0x65,
  0x73, 0x73, 0x20, 0x68, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x68, 0x65, 0x6c,
  0x70, 0x20, 0x6f, 0x72, 0x20, 0x71, 0x20, 0x74, 0x6f, 0x20, 0x71, 0x75,
  0x69, 0x74, 0x29, 0x0a};

  EXPECT_CALL(p, on_glyph(_, 1u)).WillRepeatedly(Return());
  EXPECT_CALL(p, on_glyph(StartsWith("\xe2\x80\x90"), 3u))
             .WillRepeatedly(Return());
  EXPECT_CALL(p, on_newline()).WillRepeatedly(Return());
  EXPECT_CALL(p, on_return()).WillRepeatedly(Return());

  p.parse_input((char*)input, sizeof(input) - 1);
}
