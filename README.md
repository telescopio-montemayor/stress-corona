# Simple stresser

Sweeps a stepper motor between two limit switches with variable speed and a gentle ramp.


Connections are as follows:

  - A0: Wiper of a pot between Vcc and GND. Controls speed (mapping is set via #defines).
  - 11: Stepper driver pulse input.
  - 12: Stepper driver direction input.
  - 13: Stepper driver enable input (low is enabled).
  -  8: Limit switch, normal closed.
  -  9: Limit switch output. Wired to a led, when we detect a limit hit it's active.


## Compiling

We are using [PlatformIO](http://platformio.org/) but it works fine under the Arduino IDE too.
