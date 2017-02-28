atmelFrogger
============

Frogger game for an atmel AVR ATmega324A microcontroller.

*Note:* The code in this repository was created for my 2014
CSSE2010 course for UQ, created as my major project for the
last assignment. It has not been altered since then. I was a
novice programmer at the time, and so the approach I took is
not representative of my ability at this point in time.


Installation and notes
----------------------

The code must be loaded into atmel studio and linked with the
AVR ATmega324A libraries to be compiled. I've included a .hex
direct executable to load directly into the controller.

For full functionality, the controller requires a serial connection
to a computer over USB, a SPI connection to a LED screen, and
a joystick to the analog inputs.
