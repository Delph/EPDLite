// library stuff
#include <SPI.h>

class PixelCommand
{
public:
  PixelCommand(const int16_t x, const int16_t y) :
  _x(x), _y(y) {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
  {
    PixelCommand* pc = (PixelCommand*)command;

    if (pc->_x == x && pc->_y == y)
      return input & ~(1 << (7 - (x % 8)));

    return input;
  }

  static constexpr size_t size() { return sizeof(PixelCommand); }
private:
  const int16_t _x, _y;
};

class LineCommand
{
public:
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

  static constexpr size_t size() { return sizeof(LineCommand); }
private:
  const int16_t _x0, _y0, _x1, _y1;
};

class RectCommand
{
public:
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

  static constexpr size_t size() { return sizeof(RectCommand); }
private:
  LineCommand commands[4];
};

class CircleCommand
{
public:
  CircleCommand(const int16_t x, const int16_t y, const int16_t r) :
  _x(x), _y(y), radius(r) {}

  static uint8_t process(void* command, const uint8_t input, const int16_t x, const int16_t y)
  {
    CircleCommand* cc = (CircleCommand*)command;

    const uint32_t dist = (x - cc->_x) * (x - cc->_x) + (y - cc->_y) * (y - cc->_y);
    if (cc->radius == static_cast<uint32_t>(sqrt(dist) + 0.5))
      return input & ~(1 << (7 - (x % 8)));

    return input;
  }

  static constexpr size_t size() { return sizeof(CircleCommand); }
private:
  const int16_t _x, _y;
  const int16_t radius;
};

class CommandBufferInternal
{
public:
  CommandBufferInternal() {}

  virtual size_t size() const;
  virtual size_t capacity() const;

  virtual void pop() = 0;

  virtual uint8_t process(const size_t at, const uint8_t input, const int16_t x, const int16_t y) = 0;
};

template <size_t TCommandCount, size_t TCommandSize>
class CommandBuffer : public CommandBufferInternal
{
public:
  CommandBuffer() : CommandBufferInternal(), count(0)
  {
  }

  virtual size_t size() const { return count; }
  virtual size_t capacity() const { return TCommandCount; }

  template <typename TCommand>
  void push(TCommand command)
  {
    static_assert(sizeof(TCommand) <= TCommandSize);
    memcpy(&commands[TCommandSize * count], &command, sizeof(TCommand));
    call_table[count++] = &TCommand::process;
  }

  virtual void pop()
  {
    if (count > 0)
      --count;
  }

  virtual uint8_t process(const size_t at, const uint8_t input, const int16_t x, const int16_t y) override
  {
    return (call_table[at])((void*)&commands[at * TCommandSize], input, x, y);
  }

private:
  uint8_t (*call_table[TCommandCount])(void* command, const uint8_t input, const int16_t x, const int16_t y);

  uint8_t commands[TCommandCount * TCommandSize];
  size_t count;
};


using pin_t =  int8_t;

class EPD
{
public:
  EPD(const int16_t w, const int16_t h, const pin_t cs, const pin_t dc, const pin_t busy, const pin_t reset) :
  width(w), height(h), pin_cs(cs), pin_dc(dc), pin_busy(busy), pin_reset(reset), settings(SPISettings(2000000, MSBFIRST, SPI_MODE0))
  {}

  void init();
  void reset();

  void block();

  void render();
  void render_buffer(CommandBufferInternal& buffer);
  void clear();


  // render functions
  void hline(const int16_t x0, const int16_t x1, const int16_t y);

  const int16_t width;
  const int16_t height;
private:
  void place(const int16_t x, const int16_t y);
  void command(const uint8_t c);
  void data(const uint8_t d);
  void data(const uint8_t* const d, const size_t len);

  const pin_t pin_cs;
  const pin_t pin_dc;
  const pin_t pin_busy;
  const pin_t pin_reset;

  const SPISettings settings;

  const uint8_t DATA_ENTRY_ORDER = 0x11;

  const uint8_t Y_INC_X_INC = 0b11;
  const uint8_t UPDATE_X = 0b000;
  const uint8_t UPDATE_Y = 0b100;

  const uint8_t SOFT_RESET = 0x12;

  const uint8_t WRITE_RAM = 0x24;

  const uint8_t SET_X_SIZE = 0x44;
  const uint8_t SET_Y_SIZE = 0x45;
  const uint8_t SET_X_ADDRESS = 0x4E;
  const uint8_t SET_Y_ADDRESS = 0x4F;
  const uint8_t DISPLAY_UPDATE_CONTROL = 0x21;
  const uint8_t DISPLAY_UPDATE_SEQUENCE = 0x20;
};

void EPD::init()
{
  pinMode(pin_reset, OUTPUT);
  pinMode(pin_dc, OUTPUT);
  pinMode(pin_cs, OUTPUT);
  pinMode(pin_busy, INPUT);

  digitalWrite(pin_cs, 1);
  digitalWrite(pin_reset, 1);

  SPI.begin();

  // reset the device
  Serial.println("reset");
  reset();

  // define the data entry sequence
  Serial.println("data entry sequence");
  command(DATA_ENTRY_ORDER);
  data(Y_INC_X_INC);

  // set the display size
  Serial.println("display size");
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

  Serial.println("display control");
  command(DISPLAY_UPDATE_CONTROL);
  data(0x00); // ???
  data(0x80); // ???

  Serial.println("display address");
  // set address counters
  command(SET_X_ADDRESS);
  data(0);
  command(SET_Y_ADDRESS);
  data(0);
  data(0);

  block();
}

void EPD::reset()
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

void EPD::block()
{
  do
  {
    delay(20);
  } while (digitalRead(pin_busy));
  delay(20);
}

void EPD::render()
{
  Serial.println("clear");
  clear();

  place(width / 2, height / 2);

  command(WRITE_RAM);
  data(0x00);
  data(0x00);

  hline(16, 24, 14);
  hline(16, 24, 16);
  hline(24, 28, 24);
  hline(32, 40, 32);

  Serial.println("display update sequence");
  command(DISPLAY_UPDATE_SEQUENCE);
  block();
}

void EPD::render_buffer(CommandBufferInternal& buffer)
{
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

void EPD::clear()
{
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
}

void EPD::hline(const int16_t x0, const int16_t x1, const int16_t y)
{
  place(x0, y);

  command(WRITE_RAM);

  for (int16_t x = x0; x < x1; ++x)
    data(0xff ^ 1 << (y % 8));
}


void EPD::place(const int16_t x, const int16_t y)
{
  command(SET_X_ADDRESS);
  data(x / 8);
  command(SET_Y_ADDRESS);
  data(y & 0xff);
  data((y & 0x100) >> 8);
}


void EPD::command(const uint8_t c)
{
  SPI.beginTransaction(settings);

  digitalWrite(pin_dc, 0);
  digitalWrite(pin_cs, 0);

  SPI.transfer(c);

  digitalWrite(pin_cs, 1);

  SPI.endTransaction();
}

void EPD::data(const uint8_t d)
{
  SPI.beginTransaction(settings);

  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  SPI.transfer(d);

  digitalWrite(pin_cs, 1);

  SPI.endTransaction();
}

void EPD::data(const uint8_t* const d, const size_t len)
{
  SPI.beginTransaction(settings);

  digitalWrite(pin_dc, 1);
  digitalWrite(pin_cs, 0);

  for (size_t i = 0; i < len; ++i)
    SPI.transfer(d[i]);

  digitalWrite(pin_cs, 1);
  SPI.endTransaction();
}


// my stuff
const uint8_t EPD_CS = 4;
const uint8_t EPD_DC = 5;
const uint8_t EPD_BUSY = 6;
const uint8_t EPD_RST = 7;

EPD epd(152, 296, EPD_CS, EPD_DC, EPD_BUSY, EPD_RST);

void setup()
{
  Serial.begin(9600);
  Serial.println("init");
  epd.init();

  Serial.println("render");

  CommandBuffer<5, RectCommand::size()> buffer;
  buffer.push<LineCommand>(LineCommand(10, 10, 20, 10));
  buffer.push<LineCommand>(LineCommand(20, 10, 20, 20));
  buffer.push<RectCommand>(RectCommand(50, 50, 20, 20));
  buffer.push<PixelCommand>(PixelCommand(50, 150));
  buffer.push<CircleCommand>(CircleCommand(50, 150, 20));
  epd.render_buffer(buffer);
}

void loop()
{

}
