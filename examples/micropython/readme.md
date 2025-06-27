# SQUiXL MicroPython Examples

### Context Manager support
The SQUiXL library for MicroPython now supports context managers, so you now intitialise the library using a `with` block, which will automatically deinit() the LCD peripheral when you exit back to the REPL.

This means no hard reset or even soft reset is required between stopping and re-starting your code.

### Setting the IOMUX State
SQUiXL uses an IOMUX to share the IO between the uSD card socket and the I2S Amplifier. This means both cannnot be active at the same time. To switch btween them, or to set the IOMUX to OFF, you can use the follow function:

```
# Switch the MUX off
SQUiXL.set_iomux(SQUiXL.IOMUX_OFF)

# Switch the IOMUX on (if off) and to the uSD Card output
SQUiXL.set_iomux(SQUiXL.IOMUX_SD)

# Switch the IOMUX on (it off) and to the I2S Amp output
SQUiXL.set_iomux(SQUiXL.IOMUX_I2S)
```

Currently, no `init` or `deinit` of each peripheral will happen when you switch the IOMUX using this function (like it does in the shipping C+ firmware) so you'll need to manually deinit the I2S Amp if it's currently the active output, before switching the MUX off or to the SD card output. The same applies to the SPI connection to the uSD card, if switching away from it.  
