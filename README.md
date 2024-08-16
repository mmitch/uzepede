# uzepede - a centipede clone for uzebox

Copyright (C) 2012 by  Christian Garbs <mitch@cgarbs.de>  
licensed under the GNU GPL v3 or later

<https://github.com/mmitch/uzepede>

## compilation

These instructions are based on Debian Bookworm in 2024:

01. install the `avr-libc` package via _apt(1)_, _aptitude(1)_ or the like
02. clone or download the [uzebox source repository][1]
03. enter the `uzebox/` directory
04. compile the _uzebox_ project via `make`
05. the following tools need to be compiled additionally:
    - _gconvert_ via `cd tools/gconvert/; make; cd ../..`
    - _packrom_ via `cd tools/packrom/; make; cd ../..`
06. clone or download the [uzepede source directory][2]
07. enter the `uzepede/` directory
08. manually edit the `KERNEL_DIR` setting in `default/Makefile` to point
    to the `kernel/` directory inside your `uzebox/` directory
09. compile _uzepede_ via `cd default; make`
10. you should now have a `uzepede.uze` file in the `default/` subdirectory

[1]: https://github.com/Uzebox/uzebox
[2]: https://github.com/mmitch/uzepede

## usage

There are several ways to play _uzepede_:

- copy `uzepede.uze` to the SD card of your uzebox
- run `uzepede.uze` in an emulator, eg. [cuzebox][3]
  (on Debian, you will need to install the `libsdl2-dev` package to compile it yourself)
- you should also be able to flash `uzepede.hex` directly to your uzebox,
  but you will overwrite the uzebox bootloader by doing so

[3]: https://github.com/Jubatian/cuzebox
