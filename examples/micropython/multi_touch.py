# SQUiXL MicroPython Multi Touch Example
# MIT license; Copyright (c) 2025 Seon Rozenblum - Unexpected Maker
#
# Project home:
#   https://squixl.io

import framebuf, gc
import SQUiXL as squixl
from time import sleep_ms
import math, array

with squixl as squixl:
    buf = squixl.create_display()

    squixl.screen_init_spi_bitbanged()

    fb = framebuf.FrameBuffer(buf, 480, 480, framebuf.RGB565)
    fb.fill(0x2112)
    fb.text("Hey! It's SQUiXL in MicroPython!", 100, 50, 0xffe0)
    fb.text("Touch the screen with more than 1 finger!", 70, 100, 0xffff)

    while True:
        n, points = squixl.touch.read_points()

        if n > 0:
            # print(f"Got {n} touches...")
            # We need more than 1 touch to draw a line...  we can track up to 5 touches now
            if n > 1:
                fb.fill(0x2112)
                for i in range(0, n-1):
                    fb.line(points[i][0], points[i][1], points[i+1][0], points[i+1][1], 0xffe0)
                    # print(f"{points[i][0]},{points[i][1]} - {points[i+1][0]}, {points[i+1][1]}")
                fb.line(points[n-1][0], points[n-1][1], points[0][0], points[0][1],  0xffe0)

        sleep_ms(100)
