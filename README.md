An unofficial implementation of the Scratch project player in C.

# Requirements

This project requires:

* A C compiler (invoked with `cc`)
* [GNU Make](https://www.gnu.org/software/make/)
* [SDL2](https://libsdl.org/)
* [SOIL2](https://bitbucket.org/SpartanJ/soil2) (also requires [premake 4](http://premake.github.io/))
* [zlib](http://zlib.net)
* [cmph](http://cmph.sourceforge.net)

On OS X, you can get a C compiler and GNU Make by installing the command line developer tools. Most Linux distros likely already have them installed too. For Windows, [MinGW](http://mingw.org/) includes a C compiler and GNU Make.

On OS X, zlib is already installed. On Linux, it is very likely already installed.

For anything else that you don't have installed, if you have a package manager, try getting whatever you're missing through your package manager first. Else, you will need to install whatever libraries need yourself. Install the libraries where your compiler can find them.

### Notes

SOIL2 doesn't seems to have any automatic installation. You will need to install the header and built libsoil2.a yourself. On Linux and OS X, this is `sudo mv /<pathtoSOIL>/<OS>/libsoil2.a /usr/local/lib/libsoil2.a` and `sudo cp /<pathtoSOIL2>/src/SOIL2/SOIL2.h /usr/local/include/SOIL2.h. I don't know where they go on Windows.

This project has only been tested on OS X 10.10 and later, though I have tried to make it cross platform. Feel free to test this project on platforms that I haven't!

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

To remove all of the generated object files so that the executables get rebuilt from source on the next `make`, run:
```
make clean
```

To remove all generated files, run:
```
make spotless
```

# Behavior to Expect

Currently, the project player is mostly text based, and creates and whites out a dummy window with OpenGL. The say blocks and family output to the console, and the ask block uses the console. Here are the supported blocks:

* All of the operators
* All of the data blocks (except for show/hide)
* All of the control blocks
* Custom blocks (in "More Blocks")
* All of the motion blocks except for "if on edge, bounce" and "set rotation style" (though there is no visual effect)
* Say blocks and family (prints to console), graphics effects blocks (no visual), and size blocks.
* Volume and tempo blocks (although audio is not supported)
* "when green flag clicked" hat block and all broadcast blocks (including hat block)
* Ask block (uses console) and "answer" reporter
* Timer blocks, ([ v] of [ v]) reporter, "distance to [ v]", and "username" block (though "username" returns a null string)

All blocks that aren't implemented default to a no-op, and "noop called" will be printed to the console for debug purposes.

The blocks might not behave exactly like in Scratch 2, especially with edge cases, but they should. If you find a block that doesn't behave exactly as Scratch 2, please open an issue on the GitHub repository or, if you don't have a GitHub account, notify me on my Scratch profile (username: Znapi).

Note that the string related blocks aren't yet UTF-8 compatible. They expect characters to all be a byte long. This doesn't effect the "join" block, but the "length" and "get character of" block will not work properly with strings with characters longer than a byte.

When starting the player, expect it to print lots of very light debugging information to the console during loading before actually running the project.

### Bugs

Currently, there is a major bug in project loading that occasionally causes the player to crash.

If you find any other bugs, please report them by opening an issue on the GitHub repository or, if you don't have a GitHub account, on my Scratch profile (username: Znapi).

# Credits

check: http://libcheck.github.io/check/

cmph: http://cmph.sourceforge.net/

jsmn: http://zserge.com/jsmn.html

minizip/zlib: http://www.winimage.com/zLibDll/minizip.html

SDL2: https://libsdl.org/

SOIL2: https://bitbucket.org/SpartanJ/soil2

uthash: http://troydhanson.github.io/uthash/
