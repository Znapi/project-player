An unofficial implementation of the Scratch project player in C.

# Requirements

This project requires:

* A C compiler (invoked with `cc`)
* [GNU Make](https://www.gnu.org/software/make/)
* [zlib](http://zlib.net)
* [cmph](http://cmph.sourceforge.net)

On OS X, you can get a C compiler and GNU Make by installing the command line developer tools. Most Linux distros likely already have them installed too. For Windows, [MinGW](http://mingw.org/) includes a C compiler and GNU Make.

On OS X, zlib is already installed. On Linux, it is very likely already installed.

If you don't have zlib or cmph installed, and you have a package manager, try getting them through your package manager first. Else, you will need to install them yourself. Install them where your compiler can find them.

This project has only been tested on OS X 10.10 and later, though some effort has been put into making this project cross platform. Feel free to help make this project cross platform!

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

# Credits

check: http://libcheck.github.io/check/

cmph: http://cmph.sourceforge.net/

jsmn: http://zserge.com/jsmn.html

minizip/zlib: http://www.winimage.com/zLibDll/minizip.html

SDL2: https://libsdl.org/

SOIL2: https://bitbucket.org/SpartanJ/soil2

uthash: http://troydhanson.github.io/uthash/
