# glyph = [
# 0b00011100,
# 0b00000000,
# 0b00000000,
# 0b00000000,
# 0b01000001
# ]
glyph = [
0b01111100,
0b00010010,
0b00010001,
0b00010010,
0b01111100
]

class TextCommand:
  def __init__(self, x, y, text):
    self.x = x
    self.y = y
    self.text = text

    self.offx = 0
    self.offy = 0


  def process(self, data, x, y):
    # out of x-bounds
    if x < self.x or x >= self.x + (7 + 1) * len(self.text):
      # print(f'out of x-bounds: x; {x}, self.x: {self.x}')
      return data

    # out of y-bounds
    if y < self.y or y >= self.y + 5:
      # print(f'out of y-bounds: y; {y}, self.y: {self.y}')
      return data

    # 1px letter spacing
    if (x - self.x + 1) % (7 + 1) == 0:
      return data

    print(f'{x}, {y}')
    print(f'org: {format(data, "08b")}')

    index = (x - self.x) // (7 + 1)
    print(index)
    if index >= len(self.text):
      return data
    c = self.text[index]

    glyph_slice = glyph[(y - self.y) % (5)]
    print(f'gls: {format(glyph_slice, "08b")}')
    print(f'sel: {format(1 << (((x - self.x) % (7 + 1))), "08b")}')
    print(f'bit: {format(glyph_slice & (1 << (((x - self.x) % (7 + 1)))), "08b")} {"<" if glyph_slice & (1 << ((x - self.x) % (7 + 1))) else ""}')
    data = data | ((glyph_slice & (1 << (((x - self.x) % (7 + 1))))) << (self.x % (7 + 1))) % 0xff
    print(f'dat: {format(data, "08b")}')

    return data


tc = TextCommand(0, 0, 'AA')
WIDTH = 16
HEIGHT = 8
buf = [0 for _ in range(((WIDTH+1) // 2) * HEIGHT)]

for y in range(0, HEIGHT):
  for x in range(0, WIDTH, 8):
    for xi in range(0, 8):
      buf[y * WIDTH // 8 + (x + xi) // 8] = tc.process(buf[y * WIDTH // 8 + (x + xi) // 8], x + xi, y)


for y in range(0, HEIGHT):
  for x in range(0, WIDTH, 8):
    cell = buf[y * WIDTH // 8 + x // 8]
    for xi in range(0, 8):
      if cell & (1 << xi):
        print('#', end='')
      else:
        print('.', end='')
  print('')
