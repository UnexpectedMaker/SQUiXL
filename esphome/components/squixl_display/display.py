from esphome import pins
import esphome.codegen as cg
from esphome.components import display
from esphome.components.esp32 import VARIANT_ESP32S3, only_on_variant
from esphome.components.mipi import COLOR_ORDERS, flatten_sequence, map_sequence
from esphome.components.tca9555 import TCA9555Component
import esphome.config_validation as cv
from esphome.const import (
    CONF_BLUE,
    CONF_COLOR_ORDER,
    CONF_DATA_PINS,
    CONF_DIMENSIONS,
    CONF_ENABLE_PIN,
    CONF_GREEN,
    CONF_HEIGHT,
    CONF_HSYNC_PIN,
    CONF_ID,
    CONF_IGNORE_STRAPPING_WARNING,
    CONF_INIT_SEQUENCE,
    CONF_INVERT_COLORS,
    CONF_LAMBDA,
    CONF_NUMBER,
    CONF_RED,
    CONF_VSYNC_PIN,
    CONF_WIDTH,
)

CONF_EXPANDER_ID = "expander_id"
CONF_EXP_RST_PIN = "exp_rst_pin"
CONF_EXP_MOSI_PIN = "exp_mosi_pin"
CONF_EXP_CLK_PIN = "exp_clk_pin"
CONF_EXP_CS_PIN = "exp_cs_pin"
CONF_DE_PIN = "de_pin"
CONF_PCLK_PIN = "pclk_pin"
CONF_PCLK_FREQUENCY = "pclk_frequency"
CONF_PCLK_INVERTED = "pclk_inverted"
CONF_HSYNC_PULSE_WIDTH = "hsync_pulse_width"
CONF_HSYNC_BACK_PORCH = "hsync_back_porch"
CONF_HSYNC_FRONT_PORCH = "hsync_front_porch"
CONF_VSYNC_PULSE_WIDTH = "vsync_pulse_width"
CONF_VSYNC_BACK_PORCH = "vsync_back_porch"
CONF_VSYNC_FRONT_PORCH = "vsync_front_porch"

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["mipi_rgb", "tca9555"]

squixl_ns = cg.esphome_ns.namespace("squixl_display")
SQUiXLDisplay = squixl_ns.class_(
    "SQUiXLDisplay", display.Display, cg.Component
)

DATA_PIN_SCHEMA = pins.internal_gpio_output_pin_schema


def data_pin_validate(value):
    if not isinstance(value, dict):
        try:
            return DATA_PIN_SCHEMA(
                {CONF_NUMBER: value, CONF_IGNORE_STRAPPING_WARNING: True}
            )
        except cv.Invalid:
            pass
    return DATA_PIN_SCHEMA(value)


def data_pin_set(length):
    return cv.All(
        [data_pin_validate],
        cv.Length(min=length, max=length, msg=f"Exactly {length} data pins required"),
    )


# SQUiXL default init sequence — matches st7701s_init_commands from SQUiXL DevOS
# fmt: off
SQUIXL_DEFAULT_INIT = (
    (0x11, 0x00),
    ("delay 120ms",),
    (0xFF, 0x77, 0x01, 0x00, 0x00, 0x10),
    (0xC0, 0x3B, 0x00),
    (0xC1, 0x0D, 0x02),
    (0xC2, 0x81, 0x08),
    (0xCD, 0x08),
    (0xB0, 0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08, 0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18),
    (0xB1, 0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08, 0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18),
    (0xFF, 0x77, 0x01, 0x00, 0x00, 0x11),
    (0xB0, 0x60),
    (0xB1, 0x27),
    (0xB2, 0x87),
    (0xB3, 0x80),
    (0xB5, 0x49),
    (0xB7, 0x85),
    (0xB8, 0x21),
    (0xC1, 0x78),
    (0xC2, 0x78),
    ("delay 20ms",),
    (0xE0, 0x00, 0x1B, 0x02),
    (0xE1, 0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x44, 0x44),
    (0xE2, 0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00, 0xEC, 0xA0, 0x00, 0x00),
    (0xE3, 0x00, 0x00, 0x11, 0x11),
    (0xE4, 0x44, 0x44),
    (0xE5, 0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0, 0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0),
    (0xE6, 0x00, 0x00, 0x11, 0x11),
    (0xE7, 0x44, 0x44),
    (0xE8, 0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0, 0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0),
    (0xEB, 0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40),
    (0xEC, 0x3C, 0x00),
    (0xED, 0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA),
    (0xFF, 0x77, 0x01, 0x00, 0x00, 0x00),
    (0x36, 0x00),
    (0x3A, 0x66),
    (0x21, 0x00),
    ("delay 180ms",),
    (0x29, 0x00),
    ("delay 10ms",),
)
# fmt: on


def _convert_default_init():
    """Convert the default init tuples to the format expected by map_sequence/flatten_sequence."""
    result = []
    for entry in SQUIXL_DEFAULT_INIT:
        if len(entry) == 1 and isinstance(entry[0], str):
            result.append(map_sequence(entry[0]))
        else:
            result.append(map_sequence(list(entry)))
    return result


CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        cv.Schema(
            {
                cv.GenerateID(): cv.declare_id(SQUiXLDisplay),
                cv.Required(CONF_EXPANDER_ID): cv.use_id(TCA9555Component),
                cv.Optional(CONF_DIMENSIONS, default="480x480"): cv.dimensions,
                cv.Optional(CONF_EXP_RST_PIN, default=1): cv.int_range(
                    min=0, max=15
                ),
                cv.Optional(CONF_EXP_MOSI_PIN, default=2): cv.int_range(
                    min=0, max=15
                ),
                cv.Optional(CONF_EXP_CLK_PIN, default=3): cv.int_range(
                    min=0, max=15
                ),
                cv.Optional(CONF_EXP_CS_PIN, default=4): cv.int_range(
                    min=0, max=15
                ),
                cv.Optional(CONF_INIT_SEQUENCE): cv.ensure_list(map_sequence),
                cv.Optional(CONF_COLOR_ORDER, default="BGR"): cv.one_of(
                    *COLOR_ORDERS.keys(), upper=True
                ),
                cv.Optional(CONF_INVERT_COLORS, default=False): cv.boolean,
                cv.Optional(CONF_PCLK_FREQUENCY, default="12MHz"): cv.All(
                    cv.frequency, cv.Range(min=4e6, max=30e6)
                ),
                cv.Optional(CONF_PCLK_INVERTED, default=True): cv.boolean,
                cv.Optional(CONF_HSYNC_PULSE_WIDTH, default=1): cv.int_,
                cv.Optional(CONF_HSYNC_BACK_PORCH, default=2): cv.int_,
                cv.Optional(CONF_HSYNC_FRONT_PORCH, default=2): cv.int_,
                cv.Optional(CONF_VSYNC_PULSE_WIDTH, default=2): cv.int_,
                cv.Optional(CONF_VSYNC_BACK_PORCH, default=8): cv.int_,
                cv.Optional(CONF_VSYNC_FRONT_PORCH, default=20): cv.int_,
                cv.Required(CONF_DATA_PINS): cv.Any(
                    data_pin_set(16),
                    cv.Schema(
                        {
                            cv.Required(CONF_RED): data_pin_set(5),
                            cv.Required(CONF_GREEN): data_pin_set(6),
                            cv.Required(CONF_BLUE): data_pin_set(5),
                        }
                    ),
                ),
                cv.Required(CONF_DE_PIN): pins.internal_gpio_output_pin_schema,
                cv.Required(CONF_PCLK_PIN): pins.internal_gpio_output_pin_schema,
                cv.Required(CONF_HSYNC_PIN): pins.internal_gpio_output_pin_schema,
                cv.Required(CONF_VSYNC_PIN): pins.internal_gpio_output_pin_schema,
                cv.Optional(CONF_ENABLE_PIN): cv.ensure_list(
                    pins.gpio_output_pin_schema
                ),
            }
        )
    ),
    cv.only_on_esp32,
    only_on_variant(supported=[VARIANT_ESP32S3]),
)


async def to_code(config):
    (width, height) = config[CONF_DIMENSIONS]
    var = cg.new_Pvariable(config[CONF_ID], width, height)
    await display.register_display(var, config)

    # IO expander for SPI init
    expander = await cg.get_variable(config[CONF_EXPANDER_ID])
    cg.add(var.set_expander(expander))
    cg.add(var.set_exp_rst_pin(config[CONF_EXP_RST_PIN]))
    cg.add(var.set_exp_mosi_pin(config[CONF_EXP_MOSI_PIN]))
    cg.add(var.set_exp_clk_pin(config[CONF_EXP_CLK_PIN]))
    cg.add(var.set_exp_cs_pin(config[CONF_EXP_CS_PIN]))

    # Init sequence — use custom if provided, otherwise SQUiXL default
    if CONF_INIT_SEQUENCE in config:
        raw_sequence = config[CONF_INIT_SEQUENCE]
    else:
        raw_sequence = _convert_default_init()
    cg.add(var.set_init_sequence(list(flatten_sequence(raw_sequence))))

    # Display settings
    cg.add(var.set_color_mode(COLOR_ORDERS[config[CONF_COLOR_ORDER]]))
    cg.add(var.set_invert_colors(config[CONF_INVERT_COLORS]))
    cg.add(var.set_pclk_frequency(config[CONF_PCLK_FREQUENCY]))
    cg.add(var.set_pclk_inverted(config[CONF_PCLK_INVERTED]))
    cg.add(var.set_hsync_pulse_width(config[CONF_HSYNC_PULSE_WIDTH]))
    cg.add(var.set_hsync_back_porch(config[CONF_HSYNC_BACK_PORCH]))
    cg.add(var.set_hsync_front_porch(config[CONF_HSYNC_FRONT_PORCH]))
    cg.add(var.set_vsync_pulse_width(config[CONF_VSYNC_PULSE_WIDTH]))
    cg.add(var.set_vsync_back_porch(config[CONF_VSYNC_BACK_PORCH]))
    cg.add(var.set_vsync_front_porch(config[CONF_VSYNC_FRONT_PORCH]))

    # RGB data pins
    dpins = []
    if CONF_RED in config[CONF_DATA_PINS]:
        red_pins = config[CONF_DATA_PINS][CONF_RED]
        green_pins = config[CONF_DATA_PINS][CONF_GREEN]
        blue_pins = config[CONF_DATA_PINS][CONF_BLUE]
        dpins.extend(blue_pins)
        dpins.extend(green_pins)
        dpins.extend(red_pins)
        # swap bytes to match big-endian format
        dpins = dpins[8:16] + dpins[0:8]
    else:
        dpins = config[CONF_DATA_PINS]
    for index, pin in enumerate(dpins):
        data_pin = await cg.gpio_pin_expression(pin)
        cg.add(var.add_data_pin(data_pin, index))

    # Sync pins
    pin = await cg.gpio_pin_expression(config[CONF_DE_PIN])
    cg.add(var.set_de_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_PCLK_PIN])
    cg.add(var.set_pclk_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_HSYNC_PIN])
    cg.add(var.set_hsync_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_VSYNC_PIN])
    cg.add(var.set_vsync_pin(pin))

    # Enable pins (e.g. backlight)
    if enable_pin := config.get(CONF_ENABLE_PIN):
        enable = [await cg.gpio_pin_expression(p) for p in enable_pin]
        cg.add(var.set_enable_pins(enable))

    # Lambda
    if lamb := config.get(CONF_LAMBDA):
        lambda_ = await cg.process_lambda(
            lamb, [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
