import framebuf, gc
import SQUiXL as squixl
from time import sleep_ms
import math

buf = squixl.create_display()

squixl.screen_init_spi_bitbanged()

fb = framebuf.FrameBuffer(buf, 480, 480, framebuf.RGB565)
fb.fill(0x2112)
fb.text("Hey! It's SQUiXL in MicroPython!", 100, 50, 0xffe0)
fb.ellipse(180, 200, 40, 50, 0x0, True)
fb.ellipse(190, 190, 20, 20, 0xffff, True)
fb.ellipse(300, 200, 40, 50, 0x0, True)
fb.ellipse(310, 190, 20, 20, 0xffff, True)
fb.ellipse(240, 240, 150, 150, 0xffff)
fb.rect(190, 320, 100, 5, 0xffff, True)

fb.text("RED", 100, 400, 0xf800)
fb.text("GREEN", 200, 400, 0x07e0)
fb.text("BLUE", 300, 400, 0x001f)

# sleep_ms(2000)

# NUM_COLORS = 1 << 16           # 65 536
# GRID_SIZE  = int(math.ceil(math.sqrt(NUM_COLORS)))  # 256
# CELL_SIZE  = 480 // GRID_SIZE  # 1 pixel

# for color in range(NUM_COLORS):
#     row = color // GRID_SIZE
#     col = color %  GRID_SIZE
#     x = col * CELL_SIZE
#     y = row * CELL_SIZE
#     # draw a 1×1 “rect” (i.e. single pixel) in this colour
#     fb2.fill_rect(x, y, CELL_SIZE, CELL_SIZE, color)
    

# W, H = 480, 480
# STEP      = 2                      # skip every other colour
# TOTAL     = (1 << 16) // STEP     # 65 536 / 2 = 32 768 rects
# GRID      = int(TOTAL ** 0.5)     # ≈181
# if GRID * GRID < TOTAL:
#     GRID += 1                     # ceil to cover all

# # ─── Draw filled rects in index order ─────────────────────────────────────
# for idx, colour in enumerate(range(0, 1 << 16, STEP)):
#     row = idx // GRID
#     col = idx %  GRID

#     # exact cell bounds so they sum to 480
#     x0 = col * W // GRID
#     y0 = row * H // GRID
#     x1 = (col + 1) * W // GRID
#     y1 = (row + 1) * H // GRID

#     fb2.rect(x0, y0, x1 - x0, y1 - y0, colour, True)
    
# fb.blit(fb2, 0, 0)
# # lcd.blit()

while True:
    n, points = squixl.touch.read_points()
    for i in range(0, n):
        print(f"id {points[i][3]} x {points[i][0]} y {points[i][1]} size {points[i][2]}")
    sleep_ms(100)
