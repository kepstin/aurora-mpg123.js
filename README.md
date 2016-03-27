aurora-mpg123.js
================

A port of the [mpg123](http://mpg123.de/) mp3 decoder library to JavaScript
using [emscripten](http://emscripten.org/) and the
[aurora.js framework](http://audiocogs.org/codecs/)

aurora-mpg123.js uses a well-tested mature mp3 decoder library, with full
support for "gapless" mp3 decoding (with sample-accurate trimming);
particularly useful if you're trying to have looping audio playback.
The browser version comes in at around 100KiB when gzipped.

Why use aurora-mpg123.js instead of mp3.js?
-------------------------------------------

The main reason to use aurora-mpg123.js instead of the audiocogs mp3.js
library is that it has support for gapless mp3 decoding. If you don't need
that, mp3.js might be a better option as it's a smaller download (about
half the size).

I have yet to do any benchmarking on the relative performance of the two
libraries. The mpg123 library used as the core of aurora-mpg123.js is known
to be one of the fastest mp3 decoders around, but that relies on assembly
SIMD code on modern systems, which is obviously not available here. The
precompilation of asm.js code may result in a speed improvement.

How to build aurora-mpg123.js
-----------------------------

Before you can build aurora-mpg123.js, you need to have a working emscripten
development environment. Follow the [emscripten Download and install guide]
(http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html)
and make sure that the environment is loaded. (You should be able to run
`emcc` and `emconfigure` on the command line.)

This git repository uses a submodule to manage the code from the upstream
mpg123 library. To clone the git repository, do the following:

```
git clone https://github.com/kepstin/aurora-mpg123.js.git
cd aurora-mpg123.js
git submodule update --init
```

The source tarballs downloadable from the GitHub releases page include the
corresponding mpg123 source code.

If you are doing a browser build, you have to install npm modules to get the
browserify tool used to bundle the source files. In the source directory, run:

```
npm install
```

To build, use make. Depending on how you intend to use the library, you'll
select a different make target.

To build only the libraries, for use e.g. in npm:
```
make lib
```

To build the browser bundle file:
```
make browser
```

The browser bundle file will be output as build/mpg123.js
