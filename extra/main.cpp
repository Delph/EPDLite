#include <iostream>
#include <iomanip>

#include "stub.h"
#include "../src/EPDLite/commands.h"
#include "../src/EPDLite/fonts/font5x7.h"

const int16_t WIDTH = 24;
const int16_t HEIGHT = 24;

int main()
{
  EPDLite epd(WIDTH, HEIGHT);
  epd.setOrientation(1);

  RectCommand* commands[] = {
    new RectCommand(2, 6, 2, 4, true),
    // new TextCommand(0, 8, "l", font5x7, 1),
    // new TextCommand(8, 16, "l", font5x7, 2),
  };
  uint8_t buf[((WIDTH+1) / 8) * HEIGHT] = {0};

  for (int16_t y = 0; y < HEIGHT; ++y)
  {
    for (int16_t x = 0; x < WIDTH; x += 8)
    {
      buf[y * WIDTH / 8 + x / 8] = 0xff;
      for (int16_t xi = 0; xi < 8; ++xi)
      {
        if (x + xi >= WIDTH)
          break;

        for (RectCommand* command : commands)
          buf[y * WIDTH / 8 + (x + xi) / 8] = RectCommand::process(command, buf[y * WIDTH / 8 + x / 8], x + xi, y, epd);
      }
    }
  }

  for (int16_t x = 5; x < WIDTH; x += 5)
    std::cout << std::setw(10) << x;
  std::cout << std::endl;
  for (int16_t y = 0; y < HEIGHT; ++y)
  {
    for (int16_t x = 0; x < WIDTH; x += 8)
    {
      const uint8_t cell = buf[y * WIDTH / 8 + x  / 8];
      for (int16_t xi = 0; xi < 8; ++xi)
      {
        if (!(cell & (1 << (7 - xi))))
          std::cout << "#";
        else
          std::cout << ".";
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
  for (int16_t x = 5; x < WIDTH; x += 5)
    std::cout << std::setw(10) << x;
  std::cout << std::endl;

  for (RectCommand* tc : commands)
    delete tc;
}

