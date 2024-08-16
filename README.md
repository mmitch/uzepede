# uzepede - a centipede clone for uzebox

Copyright (C) 2012 by  Christian Garbs <mitch@cgarbs.de>  
licensed under the GNU GPL v3 or later

[uzepede project page](https://github.com/mmitch/uzepede)

## To compile

- get the uzebox source from <http://code.google.com/p/uzebox/>
- change KERNEL_DIR in default/Makefile to point to kernel/ inside
  your uzebox sources
- cd default; make

## To use

- either flash it to your uzebox
- or compile the uzem emulator from the uzebox tools directory,
  then run "uzem uzepede.hex" with the proper paths
