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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    PixelCommand* pc = (PixelCommand*)command;

    if (pc->_x == x && pc->_y == y)
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    LineCommand* lc = (LineCommand*)command;
    // horizontal line
    if (lc->_y0 == lc->_y1 && lc->_y0 == y)
    {
      if (x < lc->_x0)
        return input;
      else if (x > lc->_x1)
        return input;

      return input & ~(1 << (7 - (x % 8)));
    }
    // vertical line
    else if (lc->_x0 == lc->_x1 && lc->_x0 == x)
    {
      if (y < lc->_y0)
        return input;
      if (y > lc->_y1)
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
  RectCommand(const int16_t x, const int16_t y, const int16_t width, const int16_t height) : commands{
    LineCommand(x, y, x+width, y),
    LineCommand(x+width, y, x+width, y+height),
    LineCommand(x, y+height, x+width, y+height),
    LineCommand(x, y, x, y+height)
  } {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    RectCommand* rc = (RectCommand*)command;

    uint8_t d = input;
    for (uint8_t i = 0; i < 4; ++i)
      d = LineCommand::process(&(rc->commands[i]), d, x, y, orientation);
    return d;
  }

private:
  LineCommand commands[4];
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
  CircleCommand(const int16_t x, const int16_t y, const int16_t r) :
  _x(x), _y(y), radius(r) {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    CircleCommand* cc = (CircleCommand*)command;

    const uint32_t dist = (x - cc->_x) * (x - cc->_x) + (y - cc->_y) * (y - cc->_y);
    if (cc->radius == static_cast<int16_t>(sqrt(dist) + 0.5))
      return input & ~(1 << (7 - (x % 8)));

    return input;
  }

private:
  const int16_t _x, _y;
  const int16_t radius;
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    TextCommand* tc = (TextCommand*)command;
    const char* const text = tc->txt;
    const Font& font = tc->fnt;

    if (tc->out_of_bounds(x, y, orientation))
      return input;

    const int16_t index = (orientation % 2 ? (y - tc->_y) : (x - tc->_x)) / ((font.charwidth + 1) * tc->fontsize);
    if (index >= tc->length)
      return input;

    const char c = text[index];

    return tc->render_char(input, c, x, y, orientation);
  }

private:
  bool out_of_bounds(const int16_t x, const int16_t y, const uint8_t orientation)
  {
    const char* const text = this->txt;
    const Font& font = this->fnt;

    if (orientation % 2 == 0)
    {
      // out of x-bounds
      if (x < this->_x || x >= this->_x + (font.charwidth + 1) * this->fontsize * this->length)
        return true;

      // out of y-bounds
      if (y < this->_y || y > this->_y + font.charheight * this->fontsize)
        return true;

      // 1px letter spacing
      if (((x - this->_x) / this->fontsize + 1) % (font.charwidth + 1) == 0)
        return true;
    }
    else if (orientation % 2 == 1)
    {
      // out of x-bounds
      if (x < this->_x || x > this->_x + font.charheight * this->fontsize)
        return true;

      // out of y-bounds
      if (y < this->_y || y >= this->_y + ((font.charwidth + 1) * this->fontsize) * this->length)
        return true;

      // 1px letter spacing
      if (((y - this->_y) / this->fontsize + 1) % (font.charwidth + 1) == 0)
        return true;
    }

    return false;
  }

  uint8_t render_char(const uint8_t input, const char c, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    const char* const text = this->txt;
    const Font& font = this->fnt;

    const int16_t diff = orientation % 2 ? (y - this->_y) : (x - this->_x);
    const uint8_t glyph_slice = pgm_read_byte(&(font.charmap[(c - font.mapoffset) * font.charwidth + modp(diff / this->fontsize, font.charwidth + 1)]));

    if (orientation == 0)
    {
      if ((glyph_slice >> ((y - this->_y) / this->fontsize)) & 1)
        return input & ~(1 << (x % 8));
    }
    else if (orientation == 1)
    {
      if ((glyph_slice << ((x - this->_x) / fontsize)) & 0b10000000)
        return input & ~(1 << (7 - x % 8));
    }
    else if (orientation == 3)
    {
      if ((glyph_slice << ((x - this->_x) / fontsize)) & 0b01000000)
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const uint8_t orientation)
  {
    BufferCommand* bc = (BufferCommand*)command;

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
