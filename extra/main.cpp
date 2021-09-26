#include <iostream>

#include "stub.h"
#include "../src/EPDLite/commands.h"
#include "../src/EPDLite/fonts/font5x7.h"

const int16_t WIDTH = 152;
const int16_t HEIGHT = 50;

int main()
{
  EPDLite epd(WIDTH, HEIGHT);
  epd.setOrientation(2);

  TextCommand* commands[] = {
    new TextCommand(WIDTH/2, 8, "Hello World", font5x7, 1),
  };
  uint8_t buf[((WIDTH+1) / 2) * HEIGHT] = {0};

  for (int16_t y = 0; y < HEIGHT; ++y)
  {
    for (int16_t x = 0; x < WIDTH; x += 8)
    {
      buf[y * WIDTH / 8 + x / 8] = 0xff;
      for (int16_t xi = 0; xi < 8; ++xi)
      {
        for (TextCommand* command : commands)
          buf[y * WIDTH / 8 + (x + xi) / 8] = TextCommand::process(command, buf[y * WIDTH / 8 + x / 8], x + xi, y, epd);
      }
    }
  }

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
      }
    }
    std::cout << std::endl;
  }

  for (TextCommand* tc : commands)
    delete tc;
}

