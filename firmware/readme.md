# MicroPython Firmware and Supporting Files
You've found the latest SQUiXL MicroPython firmware and supporting files.

To flash your SQUiXL with the supported MicroPython version, you'll need to have `esptool` installed. If you don't, follow [Espressif's instructions](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html) on how to install it.

### Put SQUiXL into download mode
Before you can install MicroPython, you need to erase the Flash on your SQUiXL.

Power it up and put it into download mode by following these steps:

- Press and hold the [BOOT] button
- Press and release the [RESET] button
- Release the [BOOT] button

Now the board is in download mode and the native USB will have enumerated as a serial device.

### Erase the existing Flash
Now you need tp open a command line/terminal window and execute the following command:

```bash
esptool.py --port PORTNAME erase_flash
```

* On Linux, the port name is usually similar to `/dev/ttyACM0`.
* On Mac, the port name is usually similar to `/dev/cu.usbmodem01`.
* On Windows, the port name is usually similar to `COM4`.

The erase may take a little while as it has to erase the entire 16MB.

### Flash MicroPython
Ensure you have downloaded `micropython.bin`, `partition-table.bin` and `bootloader.bin` into a folder, and you have navigated to that folder.

Now enter the following command remembering to switch out `PORTNAME` with the serial port you used to erase the flash above.

```bash
esptool.py --chip esp32s3 -p PORTNAME --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 bootloader.bin 0x10000 micropython.bin 0x8000 partition-table.bin
```

Once the flashing is done, SQUiXL *should* restart and boot up MicroPython.

**Remember:** MicroPython doesn't present a USB storage device like CircuitPython does. You interact with MicroPython using an IDE like [Thonny](https://thonny.org) or via the command line using [mpremote](https://docs.micropython.org/en/latest/reference/mpremote.html)
