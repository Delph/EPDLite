#include <EPDLite.h>

#include <EPDLite/fonts/font3x5.h>

const uint8_t EPD_CS = 4;
const uint8_t EPD_DC = 5;
const uint8_t EPD_BUSY = 6;
const uint8_t EPD_RST = 7;

EPDLite epd(152, 296, EPD_CS, EPD_DC, EPD_BUSY, EPD_RST);

void setup()
{
  epd.init();

  CommandBuffer<10, CommandBufferInterface::max_size()> buffer;
  buffer.push(LineCommand(10, 10, 20, 10));
  buffer.push(LineCommand(20, 10, 20, 20));
  buffer.push(RectCommand(50, 50, 20, 20));
  buffer.push(PixelCommand(50, 150));
  buffer.push(CircleCommand(50, 150, 20));
  buffer.push(TextCommand(50, 200, "Hello world!", font3x5));
  epd.render(buffer);
}

void loop()
{

}
