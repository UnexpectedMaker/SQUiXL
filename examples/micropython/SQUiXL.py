# SQUiXL Helper Library
# MIT license; Copyright (c) 2024 Seon Rozenblum - Unexpected Maker
#
# Project home:
#   https://squixl.io

# Import required libraries
from micropython import const
from machine import Pin, I2C, PWM, RGB
from max17048 import MAX17048
from lca9555 import LCA9555, OUTPUT, HIGH, LOW, INPUT
from drv2605 import DRV2605, Effect, Pause, MODE_INTTRIG
from gt911 import GT911
import time


# S3 IO
HSYNC 		= 48
VSYNC 		= 47
DE    		= 38
PCLK  		= 39
BL_PWM 		= 40

# EXP IO
BL_EN		= 0
LCD_RST		= 1
TP_RST		= 5
SOFT_PWR	= 6
MUX_SEL		= 8
MUX_EN		= 9
HAPTICS_EN	= 10
VBUS_SENSE	= 11
SD_DETECT	= 15

# Data pins in R0–R4, G0–G5, B0–B4 order
RGB_IO = [
    # B0–B4
    21, 18, 17, 16, 15,
    # G0–G5
    14, 13, 12, 11, 10, 9,
    # R0–R4
    8, 7, 6, 5, 4,
]

LCD_DELAY = 0xFF

# ST7701S init sequence
st7701s_init_commands = [
    1,   0x11,
    LCD_DELAY, 120,
    6,   0xFF, 0x77, 0x01, 0x00, 0x00, 0x10,
    3,   0xC0, 0x3B, 0x00,
    3,   0xC1, 0x0D, 0x02,
    3,   0xC2, 0x21, 0x08,
    2,   0xCD, 0x08,
    17,  0xB0, 0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08, 0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18,
    17,  0xB1, 0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08, 0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18,
    6,   0xFF, 0x77, 0x01, 0x00, 0x00, 0x11,
    2,   0xB0, 0x60,
    2,   0xB1, 0x30,
    2,   0xB2, 0x87,
    2,   0xB3, 0x80,
    2,   0xB5, 0x49,
    2,   0xB7, 0x85,
    2,   0xB8, 0x21,
    2,   0xC1, 0x78,
    2,   0xC2, 0x78,
    LCD_DELAY, 20,
    4,   0xE0, 0x00, 0x1B, 0x02,
    12,  0xE1, 0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x44, 0x44,
    13,  0xE2, 0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00, 0xEC, 0xA0, 0x00, 0x00,
    5,   0xE3, 0x00, 0x00, 0x11, 0x11,
    3,   0xE4, 0x44, 0x44,
    17,  0xE5, 0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0, 0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0,
    5,   0xE6, 0x00, 0x00, 0x11, 0x11,
    3,   0xE7, 0x44, 0x44,
    17,  0xE8, 0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0, 0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0,
    8,   0xEB, 0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40,
    3,   0xEC, 0x3C, 0x00,
    17,  0xED, 0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA,
    6,   0xFF, 0x77, 0x01, 0x00, 0x00, 0x00,
    2,   0x36, 0x00,
    2,   0x3A, 0x66,
    1,   0x21,
    LCD_DELAY, 120,
    LCD_DELAY, 120,
    1,   0x29,
    LCD_DELAY, 120,
    0
]

lcd = None

# back_light = PWM(BL_PWM, freq=6000, duty_u16=8192)
# back_light.duty_u16(32768)

# Initialize I2C bus
i2c = I2C(0, scl=Pin.board.TP_SCL, sda=Pin.board.TP_SDA)

# Create an instance of the MAX17048 class
max17048 = MAX17048(i2c)

# Create instance of tehe LCA9555 IO Expander
ioex = LCA9555(i2c)

ioex.pin_mode(LCD_RST, OUTPUT, HIGH)

# Screen backlight EN
ioex.pin_mode(BL_EN, OUTPUT, HIGH)

# Screen soft power EN
ioex.pin_mode(SOFT_PWR, OUTPUT, LOW)

ioex.pin_mode(VBUS_SENSE, INPUT)

# IO MUX - EN is Active LOW, so start it off
ioex.pin_mode(MUX_EN, OUTPUT, HIGH)
# IO MUX - Set default to I2S - LOW is SD
ioex.pin_mode(MUX_SEL, OUTPUT, HIGH)

# Haptic EN
ioex.pin_mode(HAPTICS_EN, OUTPUT, HIGH)

# Initialise the GT911 touch IC 
touch = GT911(i2c, irq_pin=3, reset_pin=5, ioex=ioex)

# Initialise the DRV2605 haptic engine
drv = DRV2605(i2c)
# Seth the driver to a basic click effect
drv.sequence[0] = Effect(1)


def screen_init_spi_bitbanged():
    """
    Bit-banged SPI over LCA9555 pins to send init commands to ST7701S display.
    ioex: instance of LCA9555
    """
    # Expander pins (0-15)
    MOSI = 2  # expander P2
    CLK  = 3  # expander P3
    CS   = 4  # expander P4

    # Configure pins as outputs
    ioex.pin_mode(MOSI, OUTPUT)
    ioex.pin_mode(CLK, OUTPUT)
    ioex.pin_mode(CS, OUTPUT)

    # Idle states
    ioex.write(CS, HIGH)
    ioex.write(CLK, LOW)
    ioex.write(MOSI, LOW)

    cmds = st7701s_init_commands

    i = 0
    while True:
        length = cmds[i]
        i += 1
        if length == 0:
            break
        if length == LCD_DELAY:
            delay_ms = cmds[i]
            i += 1
            time.sleep_ms(delay_ms)
            continue
        # Next byte is command
        cmd = cmds[i]
        i += 1
        # CS low to start transaction
        ioex.write(CS, LOW)
        # DC bit = 0 for command
        ioex.write(CLK, LOW)
        ioex.write(MOSI, 0)
        ioex.write(CLK, HIGH)
        # Send 8-bit command
        for bit in range(7, -1, -1):
            ioex.write(CLK, LOW)
            ioex.write(MOSI, (cmd >> bit) & 1)
            ioex.write(CLK, HIGH)
        # Send data bytes (length-1)
        for _ in range(length - 1):
            data_byte = cmds[i]
            i += 1
            # DC bit = 1 for data
            ioex.write(CLK, LOW)
            ioex.write(MOSI, 1)
            ioex.write(CLK, HIGH)
            # Send 8 bits of data
            for bit in range(7, -1, -1):
                ioex.write(CLK, LOW)
                ioex.write(MOSI, (data_byte >> bit) & 1)
                ioex.write(CLK, HIGH)
        # CS high to end transaction
        ioex.write(CS, HIGH)
        # small pause (optional)
        # time.sleep_us(10)
    time.sleep_ms(10)
    
def create_display():
    global lcd
    
    lcd = RGB(
        480, 480, RGB_IO,
        hsync             = HSYNC,
        vsync             = VSYNC,
        de                = DE,
        pclk              = PCLK,
        freq              = 6_000_000,  # 6 MHz
        num_fbs           = 1,           # double-buffering
        psram_trans_align = 64,
        sram_trans_align  = 8,
        bits_per_pixel    = 16,
        disp_gpio_num     = -1,          # no reset pin
        hsync_idle_low    = False,
        vsync_idle_low    = False,
        hsync_back_porch  = 10,
        hsync_front_porch = 50,
        hsync_pulse_width = 8,
        vsync_back_porch  = 8,
        vsync_front_porch = 8,
        vsync_pulse_width = 3,
        on_demand		  = False,
        fb_in_psram		  = True,
        bounce_buffer_size_px = 10 * 480,
    )
    # lcd = RGB(
    #     480, 480, RGB_IO,
    #     hsync             = HSYNC,
    #     vsync             = VSYNC,
    #     de                = DE,
    #     pclk              = PCLK,
    #     freq              = 6_000_000,  # 6 MHz
    #     num_fbs           = 1,           # double-buffering
    #     psram_trans_align = 64,
    #     sram_trans_align  = 8,
    #     bits_per_pixel    = 16,
    #     disp_gpio_num     = -1,          # no reset pin
    #     hsync_idle_low    = False,
    #     vsync_idle_low    = False,
    #     hsync_back_porch  = 10,
    #     hsync_front_porch = 50,
    #     hsync_pulse_width = 8,
    #     vsync_back_porch  = 8,
    #     vsync_front_porch = 8,
    #     vsync_pulse_width = 3,
    #     on_demand		  = False,
    #     fb_in_psram		  = True,
    #     bounce_buffer_size_px = 10 * 480,
    # )

    # Return the display buffer
    return lcd.get_buffer()
        
# Helper functions
def get_bat_voltage():
    """Read the battery voltage from the fuel gauge"""
    voltage = max17048.cell_voltage
    print(f"Bat Voltage: {voltage}V")
    return voltage


def get_state_of_charge():
    """Read the battery state of charge from the fuel gauge"""
    soc = max17048.state_of_charge
    print(f"State of Charge: {soc}%")
    return soc


def get_vbus_present():
    """Detect if VBUS (5V) power source is present"""
    return ioex.read(VBUS_SENSE) == 1

# --- Context Manager Support ---
import sys

def screen_deinit():
    global lcd
    if lcd is not None:
        try:
            lcd.deinit()
        except AttributeError:
            pass
        lcd = None

def __enter__():
    return sys.modules[__name__]

def __exit__(exc_type, exc_value, traceback):
    screen_deinit()
