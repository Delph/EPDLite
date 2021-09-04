# EPDLite
A small, simple, and lightweight ePaper display library for Arduino. Suitable for low memory microcontrollers such as the Arduino Uno/Nano (ATmega328) and ATtiny series which don't have enough RAM to store an entire screen buffer.

## Wiring
* VCC to 3v3 or 5v -- **warning**: some displays aren't 5v tolerant, some are. Check with your vendor first
* GND to GND
* DIN to your microcontroller's MOSI pin (D11 or ICSP-4)
* CLK to your microcontroller's SCK pin (D13 or ICSP-1)
* CS to any digital pin
* DC to any digital pin
* RST to any digital pin
* BUSY to any digital pin


## Usage

```cpp
#include <EPDLite.h>

// setup pins
const uint8_t EPD_CS = 4;
const uint8_t EPD_DC = 5;
const uint8_t EPD_BUSY = 6;
const uint8_t EPD_RST = 7;

// create an instance of our display
// Waveshare 2.66" display is 152 by 296 pixels.
EPDLite epd(152, 296, EPD_CS, EPD_DC, EPD_BUSY, EPD_RST);

void setup()
{
  // initialise
  epd.init();

  // create a command buffer to store five commands
  CommandBuffer<5, CommandBufferInterface::max_size()> buffer;

  // push some commands into our buffer
  buffer.push(LineCommand(10, 10, 20, 10));
  buffer.push(LineCommand(20, 10, 20, 20));
  buffer.push(RectCommand(50, 50, 20, 20));
  buffer.push(PixelCommand(50, 150));
  buffer.push(CircleCommand(50, 150, 20));

  // render it
  epd.render(buffer);
}

void loop()
{
  // nothing to do here
}
```


## Notes
This library has been developed exclusively with Waveshare's 2.66" (296x152 pixel) black/white display. Other size Waveshare displays should work.
Adafruit ePaper/eInk displays typically come with SRAM, and are not supported.
Partial refresh is not supported.
Coloured displays are not supported.
