PKG_CONFIG := pkg-config
EMCC := emcc
NPM := npm

EMCC_CFLAGS = -Os
EMCC_LDFLAGS = --llvm-lto 1 --memory-init-file 0

export PATH := $(shell npm bin):$(PATH)

.PHONY: all
all: lib browser

.PHONY: lib
lib: build/libmpg123.js

.PHONY: browser
browser: build/mpg123.js

.PHONY: clean
clean:
	rm -rf build

.PHONY: native-test
native-test: build/native-test

build/mpg123/Makefile:
	mkdir -p build/mpg123
	(cd build/mpg123 && emconfigure ../../mpg123/configure --with-cpu=generic_fpu --disable-fifo --disable-network --disable-id3v2 --disable-string --disable-layer1 --disable-layer2 --disable-shared --enable-static --disable-ntom --disable-downsample --disable-icy --disable-messages --disable-feature-report --disable-8bit --disable-16bit --disable-32bit --disable-equalizer CFLAGS="$(EMCC_CFLAGS)")

build/mpg123/src/libmpg123/.libs/libmpg123.a: build/mpg123/Makefile
	$(MAKE) -C build/mpg123

build/libmpg123.js: src/mpg123.c build/mpg123/src/libmpg123/.libs/libmpg123.a src/libmpg123-post.js
	$(EMCC) $(EMCC_CFLAGS) $(EMCC_LDFLAGS) -I build/mpg123/src/libmpg123 -I mpg123/src/libmpg123 src/mpg123.c build/mpg123/src/libmpg123/.libs/libmpg123.a -s NO_FILESYSTEM=1 -s RESERVED_FUNCTION_POINTERS=50 -s EXPORTED_FUNCTIONS="['_Mpg123Initialize','_Mpg123Decode','_Mpg123Destroy']" --post-js src/libmpg123-post.js -o $@

build/mpg123.js: index.js src/demuxer.js src/decoder.js build/libmpg123.js
	mkdir -p build
	browserify --global-transform browserify-shim \
		--bare --no-detect-globals \
		. \
		-o $@

build/native-test: src/mpg123.c src/native-test.c
	mkdir -p build
	$(CC) $^ `$(PKG_CONFIG) --cflags --libs libmpg123` -o $@
