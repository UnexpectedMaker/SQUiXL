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

Currently, I2S `init` and `deinit` is managed inside the `set_iomux` function, but no uSD card peripheral support has been added yet. Stay tuned for that.

### Basic I2S Audio
A new `pay_tone.py` example has been added to show how to set and use the I2S peripheral. It's REALLY basic for now, but it's good for showing how to make beeps and boops.

More advanced audio support will follow soon.

### Multi Touch
New MP firmware has been added that fixes multi touch tracking of up to 5 fingers, and a `multi_touch.py` example has been added to see how this works.

The `multi_touch.py` example will not work without first flashing the 29-06-2025 build of MP for SQUiXL on your device.
