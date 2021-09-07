# EPDLite
A small, simple, and lightweight ePaper display library for Arduino. Suitable for low memory microcontrollers such as the Arduino Uno/Nano (ATmega328) and ATtiny series which don't have enough RAM to store an entire screen buffer.

## Basic Example
### Wiring
* VCC to 3v3 or 5v -- **warning**: some displays aren't 5v tolerant, some are. Check with your vendor first
* GND to GND
* DIN to your microcontroller's MOSI pin (D11 or ICSP-4)
* CLK to your microcontroller's SCK pin (D13 or ICSP-1)
* CS to any digital pin
* DC to any digital pin
* RST to any digital pin
* BUSY to any digital pin


### Code
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
  CommandBuffer<5> buffer;

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

## Brief API overview
Full documentation is available **here**.

### Setup
`EPDLite.h` is the header to include. Fonts are availabale from `EPDLite/fonts/<fontname>.h`.

Use an instance of `EPDLite` to control an ePaper display.
```cpp
EPDLite epd(width, height, pin_chip_select, pin_data_command, pin_busy, pin_reset);
```
`width` and `height` are the screen sizes in pixels. All pins are required.

Initialise the display (in `setup()` or wherever appropriate).
```cpp
epd.begin();
```

### Buffered rendering (high memory usage)
Render from a RAM buffer
```cpp
uint8_t buffer[width / 8 * height] = {...};
epd.render(buffer);
```
Render a buffer from RAM. Beware this requires a 5,624 byte buffer for a 152 by 296px display.
Good for rendering intricate displays or images.
Each byte in the buffer holds data for eight pixels.

Render from a PROGMEM buffer
```cpp
uint8_t buffer[width / 8 * height] PROGMEM = {...};
epd.render_P(buffer);
```
Render a buffer from PROGMEM. This still requires 5,624 bytes to store the data, but the data can be stored in FLASH instead of RAM, reducing the memory requirement at the cost of being read only. Good for rendering hard coded images.

### Command rendering (low memory usage)
Create a command buffer
```cpp
CommandBuffer<5> buffer;
```
CommandBuffer stores a list of drawing commands to perform, such as lines, rectangles, and text. The first argument within the `<>` is the maximum number of commands that can be stored in the buffer.

Add commands to the buffer with `push()`
```cpp
buffer.push(LineCommand(x0, y0, x1, y1));
buffer.push(RectCommand(x, y , w, h));
```

Then render the command list;
```cpp
epd.render(buffer);
```


## Notes
This library has been developed exclusively with Waveshare's 2.66" (296x152 pixel) black/white display. Other size Waveshare displays should work.
Adafruit ePaper/eInk displays typically come with SRAM, and are not supported.
Partial refresh is not supported.
Coloured displays are not supported.
