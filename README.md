An unofficial implementation of the Scratch project player in C.

Requirements
------------

Currently, this project requires:

* Mac OS X (tested on 10.10 and 10.11)
* GNU Make (should be included with OS X)
* SDL2
* cmph
* check (if you want to use unit tests)
* SOIL

Ideally, it would support the common Linux environment and Windows. Feel free to help with making this project cross platform!

Building
--------

Open up a command shell and  `cd` to the directory that you installed this project at.

First, you will need to create the directory `obj/` at the root of the project. You can do this from the command line:
```
mkdir obj/
```

Then, to build everything, simply run:
```
make
```

The first lines GNU Make will output will be some warnings about `No such file or directory`, but they can be safely ignored.

Finally, run:
```
make blockhash
```

To generate the last file needed by the project player.

Rebuilding
----------

TODO

Credits
-------
uthash: http://troydhanson.github.io/uthash/

jsmn: http://zserge.com/jsmn.html

check: http://libcheck.github.io/check/

cmph: http://cmph.sourceforge.net/

SDL2: https://libsdl.org/

SOIL: http://lonesock.net/soil.html
