/**
 * @file EPDLite.cpp
 * @brief ePaper Display Interface
 */

#include "EPDLite.h"

void EPDLite::init()
{
  pinMode(pin_reset, OUTPUT);
  pinMode(pin_dc, OUTPUT);
  pinMode(pin_cs, OUTPUT);
  pinMode(pin_busy, INPUT);

  digitalWrite(pin_cs, 1);
  digitalWrite(pin_reset, 1);

  SPI.begin();

  // reset the device
  reset();

  // define the data entry sequence
  command(DATA_ENTRY_ORDER);
  data(Y_INC_X_INC);

  // set the display size
  command(SET_X_SIZE);
  // start
  data(0);
  // end
  data((((width - 1) / 8) & 0x1f)); // size in "address units" (bytes), -1 for when exact size

  command(SET_Y_SIZE);
  // start
  data(0);
  data(0);
  // end
  data(height & 0xff);
  data((height & 0x100) >> 8);

  command(DISPLAY_UPDATE_CONTROL);
  data(0x00); // ???
  data(0x80); // ???

  // set address counters
  command(SET_X_ADDRESS);
  data(0);
  command(SET_Y_ADDRESS);
  data(0);
  data(0);

  block();
}

void EPDLite::reset()
{
  // hard reset
  delay(10);
  digitalWrite(pin_reset, 0);
  delay(10);
  digitalWrite(pin_reset, 1);
  block();

  // soft reset
  command(SOFT_RESET);
  block();
}

void EPDLite::render(CommandBufferInterface& buffer)
{
  place(0, 0);

  command(WRITE_RAM);

  SPI.beginTransaction(settings);
  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  for (int16_t y = 0; y < height; ++y)
  {
    for (int16_t x = 0; x < width; x += 8)
    {
      uint8_t data = 0xff;
      for (int16_t xi = 0; xi < 8; ++xi)
      {
        for (size_t i = 0; i < buffer.size(); ++i)
          data = buffer.process(i, data, x + xi, y);
      }
      SPI.transfer(data);
    }
  }

  digitalWrite(pin_cs, 1);
  SPI.endTransaction();

  command(DISPLAY_UPDATE_SEQUENCE);
  block();
}

void EPDLite::render(const uint8_t* const buffer)
{
  place(0, 0);

  command(WRITE_RAM);

  SPI.beginTransaction(settings);
  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  for (int16_t y = 0; y < height; ++y)
  {
    for (int16_t x = 0; x < width; x += 8)
      SPI.transfer(buffer[y * width / 8 + width / 8]);
  }

  digitalWrite(pin_cs, 1);
  SPI.endTransaction();

  command(DISPLAY_UPDATE_SEQUENCE);
  block();
}

void EPDLite::render_P(const uint8_t* const buffer)
{
  place(0, 0);

  command(WRITE_RAM);

  SPI.beginTransaction(settings);
  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  for (int16_t y = 0; y < height; ++y)
  {
    for (int16_t x = 0; x < width; x += 8)
      SPI.transfer(pgm_read_byte(&(buffer[y * width / 8 + width / 8])));
  }

  digitalWrite(pin_cs, 1);
  SPI.endTransaction();

  command(DISPLAY_UPDATE_SEQUENCE);
  block();
}

void EPDLite::clear()
{
  place(0, 0);

  command(WRITE_RAM);

  SPI.beginTransaction(settings);
  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  for (int16_t y = 0; y < height; ++y)
  {
    for (int16_t x = 0; x < width / 8; ++x)
      SPI.transfer(0xff);
  }

  digitalWrite(pin_cs, 1);
  SPI.endTransaction();

  command(DISPLAY_UPDATE_SEQUENCE);
  block();
}

void EPDLite::block()
{
  do
  {
    delay(20);
  } while (digitalRead(pin_busy));
  delay(20);
}

void EPDLite::place(const int16_t x, const int16_t y)
{
  command(SET_X_ADDRESS);
  data(x / 8);
  command(SET_Y_ADDRESS);
  data(y & 0xff);
  data((y & 0x100) >> 8);
}


void EPDLite::command(const uint8_t c)
{
  SPI.beginTransaction(settings);

  digitalWrite(pin_dc, 0);
  digitalWrite(pin_cs, 0);

  SPI.transfer(c);

  digitalWrite(pin_cs, 1);

  SPI.endTransaction();
}

void EPDLite::data(const uint8_t d)
{
  SPI.beginTransaction(settings);

  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  SPI.transfer(d);

  digitalWrite(pin_cs, 1);

  SPI.endTransaction();
}

void EPDLite::data(const uint8_t* const d, const size_t len)
{
  SPI.beginTransaction(settings);

  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  for (size_t i = 0; i < len; ++i)
    SPI.transfer(d[i]);

  digitalWrite(pin_cs, 1);
  SPI.endTransaction();
}
