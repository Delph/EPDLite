#ifndef EPDLITE_FONT_H_INCLUDE
#define EPDLITE_FONT_H_INCLUDE

#include <stddef.h>
#include <stdint.h>

struct Font
{
  Font(const uint8_t* const map, const int16_t width, const size_t length, const size_t offset)
  : charmap(map)
  , charwidth(width)
  , maplength(length)
  , mapoffset(offset)
  {}

  const uint8_t* const charmap;
  const size_t mapoffset;
  const size_t maplength;

  const int16_t charwidth;
};

#endif
