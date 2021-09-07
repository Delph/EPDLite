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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
  {
    RectCommand* rc = (RectCommand*)command;

    uint8_t d = input;
    for (uint8_t i = 0; i < 4; ++i)
      d = LineCommand::process(&(rc->commands[i]), d, x, y);
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
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
  TextCommand(const int16_t x, const int16_t y, const char* const text, const Font& font) :
  _x(x), _y(y), txt(text), fnt(font)
  {
    offx = 0;
    offy = 0;
    index = 0;
  }

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
  {
    TextCommand* tc = (TextCommand*)command;

    if (x == tc->_x + tc->offx && y == tc->_y + tc->offy && tc->index < strlen(tc->txt))
    {
      const uint8_t glyph_slice = pgm_read_byte(&(tc->fnt.charmap[tc->txt[tc->index - fnt.offset] + tc->offy % font.charwidth]));

      const uint8_t data = input & ~glyph_slice;
      ++tc->offy;
      if (tc->offy % font.charwidth == 0)
        ++tc->index;
      return data;
    }

    return input;
  }

private:
  const int16_t _x, _y;
  const char* const txt;
  const Font& fnt;

  int16_t offx, offy;
  size_t index;
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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
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
