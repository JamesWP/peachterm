#include "colors.hpp"

namespace {

uint32_t Repack(std::array<uint32_t, 3> rgb) {
  if (rgb[0] > 255 || rgb[1] > 255 || rgb[2] > 255) {
    // Clamp with desaturation:
    float l = (rgb[0] * 299u + rgb[1] * 587u + rgb[2] * 114u) * 1e-3f, s = 1.f;
    if (rgb[0] > 255)
      s = std::min(s, (l - 255.f) / (l - rgb[0]));
    if (rgb[1] > 255)
      s = std::min(s, (l - 255.f) / (l - rgb[1]));
    if (rgb[2] > 255)
      s = std::min(s, (l - 255.f) / (l - rgb[2]));
    rgb[0] = static_cast<uint32_t>((rgb[0] - l) * s + l + 0.5f);
    rgb[1] = static_cast<uint32_t>((rgb[1] - l) * s + l + 0.5f);
    rgb[2] = static_cast<uint32_t>((rgb[2] - l) * s + l + 0.5f);
  }
  return (std::min(rgb[0], 255u) << 24) + (std::min(rgb[1], 255u) << 16) +
         (std::min(rgb[2], 255u) << 8) + 0xFF;
}

uint32_t Make16(uint32_t r, uint32_t g, uint32_t b) {
  return Repack({r * 255u / 31u, g * 255u / 31u, b * 255u / 31u});
}

uint8_t grayramp[24] = {1,  2,  3,  5,  6,  7,  8,  9,  11, 12, 13, 14,
                        16, 17, 18, 19, 20, 22, 23, 24, 25, 27, 28, 29};

uint8_t colorramp[6] = {0, 12, 16, 21, 26, 31};

auto generate_table() {
  std::array<uint32_t, 256> result = {
      Make16(0, 0, 0),    Make16(21, 0, 0),   Make16(0, 21, 0),
      Make16(21, 10, 0),  Make16(0, 0, 21),   Make16(21, 0, 21),
      Make16(0, 21, 21),  Make16(21, 21, 21), Make16(15, 15, 15),
      Make16(31, 10, 10), Make16(5, 31, 10),  Make16(31, 31, 10),
      Make16(10, 10, 31), Make16(31, 10, 31), Make16(5, 31, 31),
      Make16(31, 31, 31)};

  for (int n = 0; n < 216; ++n) {
    result[16 + n] = Make16(colorramp[(n / 36) % 6], colorramp[(n / 6) % 6],
                            colorramp[(n) % 6]);
  }

  for (int n = 0; n < 24; ++n) {
    result[232 + n] = Make16(grayramp[n], grayramp[n], grayramp[n]);
  }

  return result;
}
} // namespace

namespace colors {
std::array<uint32_t, 256> table = generate_table();
} // namespace colors
