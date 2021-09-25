/**
 * @file commands.h
 * @brief ePaper Display Interface commands
 * @ingroup  Commands
 * @addtogroup  Commands
 * \{
 */

#ifndef EPDLITE_COMMANDS_H_INCLUDE
#define EPDLITE_COMMANDS_H_INCLUDE

#include <string.h>
#include <stdint.h>

class EPDLite;
class Font;

int16_t orientate_x(const int16_t x, const int16_t y, const EPDLite& epd);

int16_t orientate_y(const int16_t x, const int16_t y, const EPDLite& epd);

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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

private:
  bool out_of_bounds(const int16_t x, const int16_t y, const int16_t tx, const int16_t ty, const EPDLite& epd);

  uint8_t render_char(const uint8_t input, const char c, const int16_t x, const int16_t y, const int16_t tx, const int16_t ty, const EPDLite& epd);

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

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd);

private:
  const uint8_t* const buf;
  const int16_t w;
  const bool mem;
};

#endif

/* \} */
