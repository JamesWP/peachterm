#include "parser.hpp"
#include "sample_vim.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
  p.parse_input(input, sizeof(input)-1);
}

TEST(VTParser, ControlSeq) { 
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 1u)).Times(15).WillRepeatedly(Return());

  char input[] = "\33[38;5;121msize_t\33[m length);";
  p.parse_input(input, sizeof(input)-1);
}
 
TEST(VTParser, UTF8) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 2u));

  char input[] = "\xc3\xb7";
  p.parse_input(input, sizeof(input)-1);
}

TEST(VTParser, Emoji) {
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 4u));

  char input[] = "\xF0\x9F\x98\x82";
  p.parse_input(input, sizeof(input)-1);
}

TEST(VTParser, BashPrompt)
{
  MockVTParser p;

  EXPECT_CALL(p, on_glyph(_, 1u)).WillRepeatedly(Return());

  char input[] =
      "\033]0;jpeach6@J-MacBookAir:~/gits/term/jterm\033\\"
      "\033]7;file://J-MacBookAir/home/jpeach6/gits/term/jterm\033\\";

  p.parse_input(input, sizeof(input)-1);
}

TEST(VTParser, Newline)
{
  MockVTParser p;

  EXPECT_CALL(p, on_newline());
  EXPECT_CALL(p, on_return());

  char input[] = "\r\n";

  p.parse_input(input, sizeof(input)-1);
}

TEST(VTParser, LargeSampleVim) { 
  MockVTParser p;
  
  EXPECT_CALL(p, on_glyph(_,_)).WillRepeatedly(Return());

  p.parse_input(sample_vim, sizeof(sample_vim));
}
