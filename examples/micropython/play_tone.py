import framebuf, gc
import SQUiXL as squixl
from time import sleep_ms, ticks_ms, ticks_diff, sleep
import math, array

squixl.set_iomux(squixl.IOMUX_I2S)

def create_tone(sample_rate = 22_050, duration = 0.025, frequency = 1000, volume = 100, log = True):
    tick_ms_generate_tone_start = ticks_ms()
    sample_count = int(sample_rate * duration)
    amplitude = 500 + (14000 * (volume/100)) # basic volume control between 500 and 14500

    wave = array.array("h", (0 for _ in range(sample_count)))
    for i in range(sample_count):
        wave[i] = int(amplitude * math.sin(2 * math.pi * frequency * i / sample_rate))
    
    if log:
        tick_ms_generate_tone_end = ticks_ms()
        print("Time to generate Tone", frequency, "Hz for", duration,
          "s with sample_rate", sample_rate, "=", "ms",
          ticks_diff(tick_ms_generate_tone_end, tick_ms_generate_tone_start))
    
    return wave


tone_high = create_tone(frequency = 1500, volume = 50)
tone_low = create_tone(frequency = 880, duration = 0.05, volume = 50)

play_high = True

with squixl as squixl:
    buf = squixl.create_display()

    squixl.screen_init_spi_bitbanged()

    fb = framebuf.FrameBuffer(buf, 480, 480, framebuf.RGB565)
    fb.fill(0x2112)
    fb.text("Hey! It's SQUiXL in MicroPython!", 100, 50, 0xffe0)

    while True:
        if squixl.current_iomux_state == squixl.IOMUX_I2S:
            squixl.audio_out.write(tone_high if play_high else tone_low)
            play_high = not play_high
            sleep(1)
