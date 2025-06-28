import framebuf, gc
import SQUiXL as squixl
from time import sleep_ms
import math

tone = squixl.make_tone(squixl.SAMPLE_RATE_IN_HZ, squixl.SAMPLE_SIZE_IN_BITS, squixl.TONE_FREQUENCY_IN_HZ)

squixl.set_iomux(squixl.IOMUX_I2S)

with squixl as squixl:
    buf = squixl.create_display()

    squixl.screen_init_spi_bitbanged()

    fb = framebuf.FrameBuffer(buf, 480, 480, framebuf.RGB565)
    fb.fill(0x2112)
    fb.text("Hey! It's SQUiXL in MicroPython!", 100, 50, 0xffe0)

    while True:
        if squixl.current_iomux_state == squixl.IOMUX_I2S:
            squixl.audio_out.write(tone)
