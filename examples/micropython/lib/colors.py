def rgb_to_565(r, g, b):
    """Convert 0–255 R,G,B to a 16-bit RGB565 value."""

    u16 = b >> 3
    u16 |= ((g >> 2) << 5)
    u16 |= ((r >> 3) << 11)
    return u16

BLACK = rgb_to_565(0, 0, 0)
WHITE = rgb_to_565(255, 255, 255)
GREY = rgb_to_565(50, 50, 50)
LIGHTGREY = rgb_to_565(100, 100, 100)
RED = rgb_to_565(255, 0, 0)
LIGHTRED = rgb_to_565(140, 0, 0)
PINK = rgb_to_565(250,100,100)
GREEN = rgb_to_565(0, 255, 0)
GREEN2 = rgb_to_565(50, 200, 50)
LIGHTGREEN = rgb_to_565(0, 100, 0)
DARKGREEN = rgb_to_565(0, 80, 0)
BLUE = rgb_to_565(0, 0, 255)
SQBLUE = 0x2112
DARKBLUE = rgb_to_565(0, 0, 90)
CYAN = rgb_to_565(0, 255, 255)
YELLOW = rgb_to_565(255, 255, 0)
MAGENTA = rgb_to_565(255, 0, 255)
PUSE = rgb_to_565(150, 100, 100)