#include <EPDLite.h>

#include <EPDLite/fonts/font5x7.h>

const uint8_t EPD_CS = 4;
const uint8_t EPD_DC = 5;
const uint8_t EPD_BUSY = 6;
const uint8_t EPD_RST = 7;

EPDLite epd(152, 296, EPD_CS, EPD_DC, EPD_BUSY, EPD_RST);
CommandBuffer<9> buffer;


void setup()
{
  epd.init();
  epd.setOrientation(1);

  buffer.push(LineCommand(10, 10, 20, 10));
  buffer.push(LineCommand(20, 10, 20, 20));
  buffer.push(RectCommand(50, 50, 20, 30));
  buffer.push(PixelCommand(276, 132));
  buffer.push(CircleCommand(296, 152, 20));
  buffer.push(TextCommand(20, 0, "ABCDEFGHIJKLM", font5x7, 1));
  buffer.push(TextCommand(40, 8, "NOPQRSTUVWXYZ", font5x7, 1));
  buffer.push(TextCommand(60, 16, "abcdefghijklm", font5x7, 2));
  buffer.push(TextCommand(80, 32, "nopqrstuvwxyz", font5x7, 2));
  epd.render(buffer);
}

void loop()
{

}
