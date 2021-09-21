/**
 * @file font.h
 * @brief ePaper Display Interface font definitions
 * @ingroup Fonts
 * @addtogroup Fonts
 */
#ifndef EPDLITE_FONT_H_INCLUDE
#define EPDLITE_FONT_H_INCLUDE

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Datatype for rendering text
 *
 */
struct Font
{
  /**
   * @brief Datatype for a bitmap font for rendering text
   * @details Datatype for storing information about how a font should be used to render correctly. Instances of this struct do not need to be manually constructed unless you wish to use your own fonts.
   *
   * @param map An array of the bitmap font data stored in PROGMEM
   * @param width The width in pixels of a single glyph
   * @param height The height in pixels of a single glyph
   * @param length How many characters appear in the font
   * @param offset Skip this many characters in the array, useful for cases like ASCII where codes below 32 are non-printable characters
   */
  Font(const uint8_t* const map, const int16_t width, const int16_t height, const size_t length, const size_t offset)
  : charmap(map)
  , charwidth(width)
  , charheight(height)
  , maplength(length)
  , mapoffset(offset)
  {}

  static int16_t width(const char* const text, const Font& font, int16_t fontsize)
  {
    return strlen(text) * (font.charwidth + 1) * fontsize;
  }

  const uint8_t* const charmap;
  const int16_t charwidth;
  const int16_t charheight;

  const size_t maplength;
  const size_t mapoffset;
};

#endif
