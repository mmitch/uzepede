# uzepede - a centipede clone for uzebox

Copyright (C) 2012-2024 by  Christian Garbs <mitch@cgarbs.de>  
licensed under the GNU GPL v3 or later

<https://github.com/mmitch/uzepede>

## features

This version of Centipede is solely based on fond memories of a
type-in game for the Schneider (Amstrad) CPC 664 that I played over 30
years ago.  I have never played the CPC game after that (I have not
found it yet, I think it was published in _Happy Computer_) nor have I
ever played the original Centipede.

So all in all, the closer you look, the more inaccuracies to the
original Centipede you will find ;-)

This game features:

- multiple worms that split if you don't hit their head

- mushrooms that clutter the level

- spiders that produce tails of mushrooms

- bees that rejuvenate mushrooms and get in your way at the bottom

- tactical gameplay: close in for faster kills or keep your distance
  where it's safer

## gameplay

Shoot all the things for a high score and don't get hit by anything
that moves :)

There are no levels and you only have one life.  It's kind of an
endless shooter.

## game rules

Everything that gets shot creates new mushrooms.

Mushrooms need three hits to be cleared.

Worms split unless they are shot in the head.

Worms move downward every time their path is blocked.

The spider creates new mushrooms in its column.

The bee regenerates any half-shot mushrooms it passes to full health.

## scoring

- hit a mushroom:  1
- split a worm:    3
- kill a worm:     5 + 2 per body part
- kill a spider :  7
- kill a bee:     10

## controls

- UP/DOWN/LEFT/RIGHT to move the player
- BUTTON A to shoot

### title screen

- START to start the game
- SELECT to show the credits screen
- SL + SL to enter secret test menu

### credits screen

- ANY button or movement to return to title screen

### sound test menu

- BUTTON A: fire shot sound
- BUTTON B: hit mushroom sound
- BUTTON X: hit worm head sound
- BUTTON Y: hit worm body sound
- UP:       new spider sound
- DOWN:     hit spiter sound
- LEFT:     new bee sound
- RIGHT:    kill bee sound
- SL:       game over sound
- SR:       title screen sound
- SELECT:   start game sound
- START:    return to title screen

### game over screen

- ANY button or movement to start a new game

## compilation

These instructions are based on Debian Bookworm in 2024:

1. install the `avr-libc` package via _apt(1)_, _aptitude(1)_ or the like
2. clone or download **my fork of** the [uzebox source repository][1]
   - Note that _uzepede_ will automatically try to compile both
     _gconvert_ and _packrom_ from the _uzebox_ toolset.
   - There is currently an upstream bug with video mode 5 that affects
     _uzepede_.  The bug is fixed in my fork of the _uzebox_ repo, so
     you can't use the official repo until [this pull request][2] has
     been merged.
3. clone or download the [uzepede source directory][3]
4. enter the `uzepede/` directory
5. manually edit the `UZEBOX_DIR` setting in `default/Makefile`
   to point to your `uzebox/` directory from step 2
6. compile _uzepede_ via `cd default; make`
7. you should now have a `uzepede.uze` file in the `default/` subdirectory

[1]: https://github.com/mmitch/uzebox
[2]: https://github.com/Uzebox/uzebox/pull/146
[3]: https://github.com/mmitch/uzepede

## usage

There are several ways to play _uzepede_:

- copy `uzepede.uze` to the SD card of your uzebox
- run `uzepede.uze` in an emulator, eg. [cuzebox][4]
  (on Debian, you will need to install the `libsdl2-dev` package to compile it yourself)
- you should also be able to flash `uzepede.hex` directly to your uzebox,
  but you will overwrite the uzebox bootloader by doing so

[4]: https://github.com/Jubatian/cuzebox
