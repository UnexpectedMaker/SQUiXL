# SQUiXL MicroPython Examples

### Context Manager support
The SQUiXL library for MicroPython now supports context managers, so you now intitialise the library using a `with` block, which will automatically deinit() the LCD peripheral when you exit back to the REPL.

This means no hard reset or even soft reset is required between stopping and re-starting your code.