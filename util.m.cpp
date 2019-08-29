#include "util.hpp"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace util;

TEST(DirtyTracker, Test)
{

  DirtyTracker<char> a;

  ASSERT_TRUE(a.dirty());

  a = 'a';

  ASSERT_TRUE(a.dirty());

  a.dirty() = false;
  a = 'a';

  ASSERT_FALSE(a.dirty());

  a = 'b';

  ASSERT_TRUE(a.dirty());

  a = 'a';

  ASSERT_TRUE(a.dirty());
}

TEST(DirtyTracker, Algo)
{
  std::vector<DirtyTracker<char>> char_list;
  char_list.resize(10);

  for (auto &tracking_char : char_list) {
    tracking_char = 'A';
    tracking_char.dirty() = false;
  }

  auto num_dirty = [&char_list]() {
    return std::count_if(char_list.begin(), char_list.end(),
                         [](const auto &tc) { return tc.dirty(); });
  };

  ASSERT_EQ(10, char_list.size());
  ASSERT_EQ(0,  num_dirty());

  char_list[3] = 'B';

  ASSERT_EQ(1,  num_dirty());

  char_list[3].dirty() = false;

  ASSERT_EQ(0,  num_dirty());

  std::vector<char> new_values;
  new_values.resize(10);
  std::fill(new_values.begin(), new_values.end(), 'B');

  std::copy(new_values.begin(), new_values.end(), char_list.begin());

  ASSERT_EQ(9,  num_dirty());

  for (auto &tracking_char : char_list) {
    tracking_char = 'B';
    tracking_char.dirty() = false;
  }

  std::copy(new_values.begin(), new_values.end(), char_list.begin());

  ASSERT_EQ(0, num_dirty());
}
