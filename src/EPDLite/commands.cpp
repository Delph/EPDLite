#include "commands.h"

#include "EPDLite.h"
#include "font.h"


int16_t orientate_x(const int16_t x, const int16_t y, const EPDLite& epd)
{
  switch (epd.getOrientation())
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
  switch (epd.getOrientation())
  {
    case 0:
      return y;
    case 1:
      return x;
    case 2:
      return epd.height - y;
    case 3:
      return epd.height - x;
  }
  return y;
}


uint8_t PixelCommand::process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
{
  PixelCommand* pc = (PixelCommand*)command;

  const int16_t tx = orientate_x(pc->_x, pc->_y, epd);
  const int16_t ty = orientate_y(pc->_x, pc->_y, epd);

  if (tx == x && ty == y)
    return input & ~(1 << (7 - (x % 8)));

  return input;
}

uint8_t LineCommand::process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
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

uint8_t RectCommand::process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
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

uint8_t CircleCommand::process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
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

uint8_t TextCommand::process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
{
  TextCommand* tc = (TextCommand*)command;
  const char* const text = tc->txt;
  const Font& font = tc->fnt;

  const int16_t tx = orientate_x(tc->_x, tc->_y, epd);
  const int16_t ty = orientate_y(tc->_x, tc->_y, epd);

  if (tc->out_of_bounds(x, y, tx, ty, epd))
    return input;

  const int16_t index = (epd.getOrientation() % 2 ? (y - ty) : (x - tx)) / ((font.charwidth + 1) * tc->fontsize);
  if (index >= tc->length)
    return input;

  const char c = epd.getOrientation() >= 2 ? text[tc->length - 1 - index] : text[index];

  return tc->render_char(input, c, x, y, tx, ty, epd);
}

bool TextCommand::out_of_bounds(const int16_t x, const int16_t y, const int16_t tx, const int16_t ty, const EPDLite& epd)
{
  const Font& font = this->fnt;

  if (epd.getOrientation() % 2 == 0)
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
  else if (epd.getOrientation() % 2 == 1)
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

uint8_t TextCommand::render_char(const uint8_t input, const char c, const int16_t x, const int16_t y, const int16_t tx, const int16_t ty, const EPDLite& epd)
{
  const Font& font = this->fnt;

  const int16_t diff = epd.getOrientation() % 2 ? (y - ty) : (x - tx);
  const int16_t d = abs((epd.getOrientation() > 2 ? font.charwidth - 1 : 0) - modp(diff / this->fontsize, font.charwidth + 1));
  const uint8_t glyph_slice = pgm_read_byte(&(font.charmap[(c - font.mapoffset) * font.charwidth + d]));

  if (epd.getOrientation() == 0)
  {
    if ((glyph_slice >> ((y - ty) / this->fontsize)) & 1)
      return input & ~(1 << (7 - x % 8));
  }
  else if (epd.getOrientation() == 1)
  {
    if ((glyph_slice << ((x - tx) / fontsize)) & 0b10000000)
      return input & ~(1 << (7 - x % 8));
  }
  else if (epd.getOrientation() == 2)
  {
    if ((glyph_slice << ((y - ty) / this->fontsize)) & 0b10000000)
      return input & ~(1 << (7 - x % 8));
  }
  else if (epd.getOrientation() == 3)
  {
    if ((glyph_slice >>((x - tx) / fontsize)) & 1)
      return input & ~(1 << (7 - x % 8));
  }
  return input;
}

uint8_t BufferCommand::process(void* command, const uint8_t input, const int16_t x, const int16_t y, const EPDLite& epd)
{
  BufferCommand* bc = (BufferCommand*)command;

  (void)epd;
  (void)input;

  if (bc->mem)
    return pgm_read_byte(bc->buf[y * (bc->w / 8) + x / 8]);
  return bc->buf[y * (bc->w / 8) + x / 8];
}
