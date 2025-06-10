# SQUiXL Examples for Arduino
You found the SQUiXL examples for Arduino. Though the SQUiXL board definition for Arduino IDE has been merged into the master branch, there has been no new release of the Arduino ESP32 Core yet that includes it, so for now, to use your SQUiXL in the Arduino IDE you will need to set your board up with the settings you see in the screenshot below.

Key values to be set are:

- **Board:** ESP32S3 Dev Module
- **USB CDC On Boot:** Enabled
- **Flash Size:** 16MB
- **Partition Scheme:** 16MB Flash (3MB App)
- **PSRAM:** OPI PSRAM
- **Upload Mode:** JUART0 / Hardware CDC
- **USB Mode:** Hardware CDC and JTAG


<img src="https://squixl.io/images/Temporary_Arduino_IDE_Board_Settings.jpg" style="padding:20px;"/>

Once the new `arduino ESP32 Core` release is out, assuming no compiler issues, you will be able to just select `UM SQUIXL` from the boards list.

# BB_SPI_LCD library
Please uninstall any version of BB_SPI_LCD you have installed via the library manager and manually install [my fork from GitHub](https://github.com/unexpectedmaker/bb_spi_lcd)


## SQUiXL Audio
This example show you how to switch the IOMUX to use the I2S audio system to play a pre-encoded WAV file on boot and to play different tones based on where you touch the scree.

It also shows you how to write text to the screen as well as drawling lines and circles with random colors.

## SQUiXL Photo Viewer
This example shows you how to switch the IOMUX to the uSD card slot to load JPG files from the card and show them on the screen, like a photo viewer.

It also shows you how to use the touch screen to cycle to the next image.

## SQUiXL Full Of Stars
This example is here for my own vanity - because it's just way cool, with no other purpose in terms of learning anything new... but who doesn't need a stars flying past them on their SQUiXL???