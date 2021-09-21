/**
 * @file commands.h
 * @brief ePaper Display Interface commands
 * @ingroup  Commands
 * @addtogroup  Commands
 * \{
 */

#ifndef EPDLITE_COMMANDS_H_INCLUDE
#define EPDLITE_COMMANDS_H_INCLUDE

#include "font.h"
#include "../EPDLitef.h"

int16_t orientate_x(const int16_t x, const int16_t y, const EPDLite& epd)
{
  switch (epd.orientation)
  {
    case 0:
      return x;
    case 1:
      return epd.width - y;
    case 2:
      return epd.width - x;
    case 3:
      return y;
  }
  return x;
}

int16_t orientate_y(const int16_t x, const int16_t y, const EPDLite& epd)
{
  switch (epd.orientation)
  {
    case 0:
      return y;
    case 1:
      return epd.height - x;
    case 2:
      return epd.height - y;
    case 3:
      return x;
  }
  return y;
}

/**
 * @brief Draws a single pixel onto the display
 *
 */
class PixelCommand
{
public:
  /**
   * @brief Draws a single pixel onto the display
   *
   * @param x X position of the pixel
   * @param y Y position of the pixel
   */
  PixelCommand(const int16_t x, const int16_t y) :
  _x(x), _y(y) {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
  {
    PixelCommand* pc = (PixelCommand*)command;

    const int16_t tx = orientate_x(pc->_x, pc->_y, epd);
    const int16_t ty = orientate_y(pc->_x, pc->_y, epd);

    if (tx == x && ty == y)
      return input & ~(1 << (7 - (x % 8)));

    return input;
  }

private:
  const int16_t _x, _y;
};

/**
 * @brief Draws a one width line
 * @details Only horizontal and vertical lines are currently supported
 */
class LineCommand
{
public:
  /**
   * @brief Draws a one width line
   * @details Only horizontal and vertical lines are currently supported
   *
   * @param x0 X position of the start of the line
   * @param y0 Y position of the start of the line
   * @param x1 X position of the end of the line
   * @param y1 Y position of the end of the line
   */
  LineCommand(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1) :
  _x0(x0), _y0(y0), _x1(x1), _y1(y1) {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
  {
    LineCommand* lc = (LineCommand*)command;

    const int16_t tx0 = orientate_x(lc->_x0, lc->_y0, epd);
    const int16_t ty0 = orientate_y(lc->_x0, lc->_y0, epd);
    const int16_t tx1 = orientate_x(lc->_x1, lc->_y1, epd);
    const int16_t ty1 = orientate_y(lc->_x1, lc->_y1, epd);

    // horizontal line
    if (ty0 == ty1 && ty0 == y)
    {
      if (x < tx0)
        return input;
      else if (x > tx1)
        return input;

      return input & ~(1 << (7 - (x % 8)));
    }
    // vertical line
    else if (tx0 == tx1 && tx0 == x)
    {
      if (y < ty0)
        return input;
      if (y > ty1)
        return input;

      return input & ~(1 << (7 - (x % 8)));
    }

    return input;
  }

private:
  const int16_t _x0, _y0, _x1, _y1;
};

/**
 * @brief Draws a rectangle
 */
class RectCommand
{
public:
  /**
   * @brief Draws a rectangle
   *
   * @param x X position of the top left corner
   * @param y Y position of the top left corner
   * @param width Width of the rectangle
   * @param height Height of the rectangle
   */
  RectCommand(const int16_t x, const int16_t y, const int16_t width, const int16_t height, const bool fill)
  : _x(x), _y(y), _w(width), _h(height), f(fill)
  {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
  {
    RectCommand* rc = (RectCommand*)command;

    const int16_t tx = orientate_x(rc->_x, rc->_y, epd);
    const int16_t ty = orientate_y(rc->_x, rc->_y, epd);

    if (rc->f)
    {
      if (x < tx || x > tx + rc->_w)
        return input;
      if (y < ty || y > ty + rc->_h)
        return input;

      return input & ~(1 << (7 - (x % 8)));
    }
    else
    {
      if ((x == tx || x == tx + rc->_w) && (y >= ty && y <= ty + rc->_h))
        return input & ~(1 << (7 - (x % 8)));
      if ((y == ty || y == ty + rc->_h) && (x >= tx && x <= tx + rc->_w))
        return input & ~(1 << (7 - (x % 8)));
    }
    return input;
  }

private:
  const int16_t _x;
  const int16_t _y;
  const int16_t _w;
  const int16_t _h;
  const bool f;
};

/**
 * @brief Draws a circle
 */
class CircleCommand
{
public:
  /**
   * @brief Draws a circle
   *
   * @param x X position of the circle's origin
   * @param y Y position of the circle's origin
   * @param r Radius of the circle
   */
  CircleCommand(const int16_t x, const int16_t y, const int16_t r, const bool fill) :
  _x(x), _y(y), radius(r), f(fill) {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
  {
    CircleCommand* cc = (CircleCommand*)command;

    const int16_t tx = orientate_x(cc->_x, cc->_y, epd);
    const int16_t ty = orientate_y(cc->_x, cc->_y, epd);

    // if we're too far out, don't even consider it
    if (abs(x - tx) > cc->radius + 1)
      return input;

    if (abs(y - ty) > cc->radius + 1)
      return input;

    const float dist = static_cast<float>(x - tx) * static_cast<float>(x - tx) + static_cast<float>(y - ty) * static_cast<float>(y - ty);
    if (cc->f)
    {
      const float rsq = static_cast<float>(cc->radius) * static_cast<float>(cc->radius);
      if (dist <= rsq)
        return input & ~(1 << (7 - (x % 8)));
    }
    else
    {
      if (cc->radius == floor(sqrt(dist) + 0.5f))
        return input & ~(1 << (7 - (x % 8)));
    }

    return input;
  }

private:
  const int16_t _x, _y;
  const int16_t radius;
  const bool f;
};


template <typename T>
T modp(const T a, const T b)
{
  return (((a % b) + b) % b);
}

/**
 * @brief Draws some text
 */
class TextCommand
{
public:
  /**
   * @brief Draws some text
   *
   * @param x X position of the start of the text
   * @param y Y position of the start of the text
   * @param text The text to draw
   * @param font The font to use
   */
  TextCommand(const int16_t x, const int16_t y, const char* const text, const Font& font, int16_t size) :
  _x(x), _y(y), txt(text), length(strlen(text)), fnt(font), fontsize(size)
  {
  }

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
  {
    TextCommand* tc = (TextCommand*)command;
    const char* const text = tc->txt;
    const Font& font = tc->fnt;

    const int16_t tx = orientate_x(tc->_x, tc->_y, epd);
    const int16_t ty = orientate_y(tc->_x, tc->_y, epd);

    if (tc->out_of_bounds(x, y, tx, ty, epd))
      return input;

    const int16_t index = (epd.orientation % 2 ? (y - ty) : (x - tx)) / ((font.charwidth + 1) * tc->fontsize);
    if (index >= tc->length)
      return input;

    const char c = text[index];

    return tc->render_char(input, c, x, y, tx, ty, epd);
  }

private:
  bool out_of_bounds(const int16_t x, const int16_t y, const int16_t tx, const int16_t ty, const EPDLite& epd)
  {
    const Font& font = this->fnt;

    if (epd.orientation % 2 == 0)
    {
      // out of x-bounds
      if (x < tx || x >= tx + (font.charwidth + 1) * this->fontsize * this->length)
        return true;

      // out of y-bounds
      if (y < ty || y > ty + font.charheight * this->fontsize)
        return true;

      // 1px letter spacing
      if (((x - tx) / this->fontsize + 1) % (font.charwidth + 1) == 0)
        return true;
    }
    else if (epd.orientation % 2 == 1)
    {
      // out of x-bounds
      if (x < tx || x > tx + font.charheight * this->fontsize)
        return true;

      // out of y-bounds
      if (y < ty || y >= ty + ((font.charwidth + 1) * this->fontsize) * this->length)
        return true;

      // 1px letter spacing
      if (((y - ty) / this->fontsize + 1) % (font.charwidth + 1) == 0)
        return true;
    }

    return false;
  }

  uint8_t render_char(const uint8_t input, const char c, const int16_t x, const int16_t y, const int16_t tx, const int16_t ty, const EPDLite& epd)
  {
    const Font& font = this->fnt;

    const int16_t diff = epd.orientation % 2 ? (y - ty) : (x - tx);
    const uint8_t glyph_slice = pgm_read_byte(&(font.charmap[(c - font.mapoffset) * font.charwidth + modp(diff / this->fontsize, font.charwidth + 1)]));

    if (epd.orientation == 0)
    {
      if ((glyph_slice >> ((y - ty) / this->fontsize)) & 1)
        return input & ~(1 << (x % 8));
    }
    else if (epd.orientation == 1)
    {
      if ((glyph_slice << ((x - tx) / fontsize)) & 0b10000000)
        return input & ~(1 << (7 - x % 8));
    }
    else if (epd.orientation == 3)
    {
      if ((glyph_slice << ((x - tx) / fontsize)) & 0b01000000)
        return input & ~(1 << (x % 8));
    }
    return input;
  }

  const int16_t _x, _y;
  const char* const txt;
  const int16_t length;
  const Font& fnt;
  const int16_t fontsize;
};


/**
 * @brief Draws the contents of a buffer
 * @details Using a memory or PROGMEM buffer, the contents is placed onto the screen.
 */
class BufferCommand
{
public:
  /**
   * @brief Draws the contents of a buffer
   * @details Using a memory or PROGMEM buffer, the contents is placed onto the screen.
   *
   * @param buffer The buffer to draw
   * @param width The width of the display to draw on
   * @param progmem True if the buffer is in progmem and needs to be read, false otherwise
   */
  BufferCommand(const uint8_t* const buffer, const int16_t width, const bool progmem) :
  buf(buffer), w(width), mem(progmem)
  {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
  {
    BufferCommand* bc = (BufferCommand*)command;

    (void)epd;
    (void)input;

    if (bc->mem)
      return pgm_read_byte(bc->buf[y * (bc->w / 8) + x / 8]);
    return bc->buf[y * (bc->w / 8) + x / 8];
  }
private:
  const uint8_t* const buf;
  const int16_t w;
  const bool mem;
};

#endif

/* \} */
