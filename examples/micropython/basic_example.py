import framebuf, gc
import SQUiXL as squixl
from time import sleep_ms, ticks_ms, ticks_diff
import math, array

# The SQUiXL library now supports context managers, so we now intitialise the library using a with block, which will automatically deinit() the LCD peripheral when you exit back to the REPL.

# This means no hard reset or even soft reset is required between stopping and re-starting your code.

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


beep = create_tone(frequency = 1500, duration = 0.025, volume = 10)

beeping = False

def i2s_callback(arg):
    global beeping
    beeping = False
    

squixl.audio_out.irq(i2s_callback)  

with squixl as squixl:
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

    while True:
        if not beeping:
            n, points = squixl.touch.read_points()

            if n > 0 and squixl.current_iomux_state == squixl.IOMUX_I2S:
                # beeping = True
                # squixl.audio_out.write(beep)
                print(f"Got {n} touches...")
                for i in range(0, n):
                    print(f"id:{points[i][3]} x {points[i][0]} y {points[i][1]} size {points[i][2]}")

            # else:
            sleep_ms(100)
