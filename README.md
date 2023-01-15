# PicoAcornSystem1
An Acorn System 1 emulator running on Pi Pico with Waveshare touch screen


As a base I took a simple 6502 emulator by Mike Chambers (https://codegolf.stackexchange.com/questions/12844/emulate-a-mos-6502-cpu) and I did a few modifications to emulate the 8154 at addresses &E00-&E04. I used the Waveshare demo programs as a start for the display and keyboard functions. Drawing on the display is rather slow. Because of this I do not clear the display when it is not addresses. The original System1 only has one display active but because it updates all of them often enough our human eyes don't notice that.

The timing is not very accurate and it approaches a 1 MHz processor. But since this is a fun project, just a test and proof of concept I don't really care. Also loading and saving a block of memory is not implemented.
