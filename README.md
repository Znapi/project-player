An unofficial implementation of the Scratch project player in C.

# Requirements

Currently, this project requires:

* OS X (10.10 and later tested) or possibly Linux (untested)
* GNU Make (should be included with OS X)
* SDL2 (https://libsdl.org/)
* cmph (http://cmph.sourceforge.net/)
* SOIL (http://lonesock.net/soil.html)

Ideally, this project would support Linux and Windows as well. Feel free to help with making this project cross platform!

Linux might already be supported, since both OS X and Linux are Posix compliant. Linux support just hasn't been tested yet.

# Building

Open up a command shell and  `cd` to the directory that you installed this project at.

Then, to build everything you need, simply run:
```
make
```

This should generate an executable named `player`. To run it, just run:
```
./player
```

## Cleaning

To remove all of the generated object files so that the executables have to be rebuilt from source, run:
```
make clean
```

To remove all generated files, run:
```
make spotless
```

## Top Level Targets

There are 2 different top level targets that can be built:

* `player` (default)
* `graphics_demo`

To build `player`, you just need to run `make`. Following it with `player` is optional. `player` is just the current command line only project player.

To build `graphics_demo`, run `make graphics_demo`. `graphics_demo` is a basic program to create a window and draw an image on it. It is just for testing portability.

To build both, run `make all`.

# Credits

uthash: http://troydhanson.github.io/uthash/

jsmn: http://zserge.com/jsmn.html

check: http://libcheck.github.io/check/

cmph: http://cmph.sourceforge.net/

SDL2: https://libsdl.org/

SOIL: http://lonesock.net/soil.html
