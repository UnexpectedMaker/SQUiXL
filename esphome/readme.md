# ESPHome Component for SQUiXL

This component allows SQUiXL to work with ESPHome and Home Assistant without requiring any ESPHome source file changes.

Currently supports:

- Screen display
- Single finger touch

## Requirements

- ESPHome 2024.x or later
- [SQUiXL](http://squixl.io) by Unexpected Maker

## Quick Start

Add this to your ESPHome YAML config:

```yaml
esphome:
  name: squixl
  friendly_name: SQUiXL

esp32:
  board: esp32-s3-devkitc-1
  variant: esp32s3
  flash_size: 16MB
  partitions: partitions.csv
  framework:
    type: esp-idf

psram:
  mode: octal
  speed: 80MHz

external_components:
  - source:
      type: git
      url: https://github.com/UnexpectedMaker/SQUiXL
      path: esphome/components
    components: [squixl_display]

i2c:
  sda: GPIO1
  scl: GPIO2
  frequency: 400kHz

tca9555:
  id: io_expander
  address: 0x20

output:
  - platform: ledc
    id: backlight_pwm
    pin: GPIO40

light:
  - platform: monochromatic
    output: backlight_pwm
    name: "Backlight"
    restore_mode: ALWAYS_ON

display:
  - platform: squixl_display
    id: squixl_lcd
    expander_id: io_expander
    dimensions: 480x480
    de_pin: GPIO38
    pclk_pin: GPIO39
    hsync_pin: GPIO48
    vsync_pin: GPIO47
    data_pins:
      red:
        - 4
        - 5
        - 6
        - 7
        - 8
      green:
        - 9
        - 10
        - 11
        - 12
        - 13
        - 14
      blue:
        - 15
        - 16
        - 17
        - 18
        - 21
    enable_pin:
      - tca9555: io_expander
        number: 0
        mode: output

# GT911 capacitive touch — reset handled by squixl_display component
# Do NOT set reset_pin here
touchscreen:
  - platform: gt911
    address: 0x14
    interrupt_pin: GPIO3
```

## How It Works

The SQUiXL display uses an ST7701S controller with a standard ESP32-S3 RGB parallel bus for pixel data. However, the display's SPI init lines (CS, CLK, MOSI, Reset) are wired through an LCA9555 IO expander over I2C instead of native GPIO.

This component handles that by bit-banging the SPI init sequence through the IO expander during startup, then handing off to ESPHome's standard RGB display driver for all pixel operations.

The GT911 capacitive touch controller's reset pin is also on the IO expander (pin 5). This component handles the GT911 reset with the correct INT pin state for I2C address selection. ESPHome's stock `gt911` touchscreen component is then used for touch events — do **not** set `reset_pin` in the GT911 config.

The LCA9555 is register-compatible with the TCA9555, so ESPHome's existing `tca9555` component is used directly.

## Configuration

### Required

| Option | Description |
|---|---|
| `expander_id` | ID of the `tca9555` component for the LCA9555 |
| `data_pins` | 16 RGB data pins (5 red, 6 green, 5 blue) |
| `de_pin` | Data enable pin (GPIO38) |
| `pclk_pin` | Pixel clock pin (GPIO39) |
| `hsync_pin` | Horizontal sync pin (GPIO48) |
| `vsync_pin` | Vertical sync pin (GPIO47) |

### Optional (with SQUiXL defaults)

| Option | Default | Description |
|---|---|---|
| `dimensions` | `480x480` | Display resolution |
| `exp_rst_pin` | `1` | LCA9555 pin for LCD reset |
| `exp_mosi_pin` | `2` | LCA9555 pin for LCD SPI data |
| `exp_clk_pin` | `3` | LCA9555 pin for LCD SPI clock |
| `exp_cs_pin` | `4` | LCA9555 pin for LCD SPI CS |
| `exp_touch_rst_pin` | `5` | LCA9555 pin for GT911 touch reset |
| `touch_int_pin` | `3` | ESP32-S3 GPIO for GT911 interrupt |
| `pclk_frequency` | `12MHz` | Pixel clock frequency |
| `pclk_inverted` | `true` | Invert pixel clock |
| `color_order` | `BGR` | Color order (RGB or BGR) |
| `invert_colors` | `false` | Invert display colors |
| `init_sequence` | SQUiXL default | Custom ST7701S init sequence |
| `enable_pin` | none | Pin(s) to drive high during setup (e.g. backlight enable) |

### Timing (with SQUiXL defaults)

| Option | Default |
|---|---|
| `hsync_pulse_width` | `1` |
| `hsync_back_porch` | `2` |
| `hsync_front_porch` | `2` |
| `vsync_pulse_width` | `2` |
| `vsync_back_porch` | `8` |
| `vsync_front_porch` | `20` |

## Partition Table

The `partitions.csv` file included in this component matches the SQUiXL factory partition layout. Reference it in your ESP32 config to avoid overwriting existing flash data:

```yaml
esp32:
  flash_size: 16MB
  partitions: partitions.csv
```

## LCA9555 IO Expander Pin Reference

| Pin | Function |
|---|---|
| 0 | Backlight Enable |
| 1 | LCD Reset |
| 2 | LCD SPI Data |
| 3 | LCD SPI Clock |
| 4 | LCD SPI CS |
| 5 | Touch IC Reset |
| 7 | uSD Card Detect |
| 8 | IOMUX Select |
| 9 | IOMUX Enable |
| 10 | Haptics Enable |
| 11 | VBUS Sense |
