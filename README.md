An unofficial implementation of the Scratch project player in C.

# Requirements

Currently, this project requires:

* GNU Make (probably already installed, comes with OS X)
* SDL2 (https://libsdl.org/)
* cmph (http://cmph.sourceforge.net/)
* SOIL (http://lonesock.net/soil.html)
* zlib (likely already installed, comes with OS X)

This project has only been tested on OS X 10.10 and later. Whether or not it can be built and run on Linux and Windows(with MinGW or Cygwin) is unknown. Feel free to test and help make this project cross platform!

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

minizip/zlib: http://www.winimage.com/zLibDll/minizip.html