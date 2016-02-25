PKG_CONFIG := pkg-config
EMCC := emcc
EMCC_CFLAGS = -O3
EMCC_LDFLAGS = --llvm-lto 1 --memory-init-file 0

.PHONY: all
all: build/mpg123.emcc.js

.PHONY: clean
clean:
	rm -rf build

.PHONY: mpg123
mpg123: build/mpg123/Makefile
	$(MAKE) -C build/mpg123

.PHONY: native-test
native-test: build/native-test

build/mpg123/Makefile:
	mkdir -p build/mpg123
	(cd build/mpg123 && emconfigure ../../mpg123/configure --with-cpu=generic_fpu --disable-fifo --disable-network --disable-layer1 --disable-layer2 --disable-shared --enable-static --disable-ntom --disable-downsample --disable-icy --disable-messages --disable-feature-report --disable-8bit --disable-16bit --disable-32bit --disable-equalizer CFLAGS="$(EMCC_CFLAGS)")

build/native-test: src/mpg123.c src/native-test.c
	mkdir -p build
	$(CC) $^ `$(PKG_CONFIG) --cflags --libs libmpg123` -o $@

build/mpg123.emcc.js: src/mpg123.c mpg123
	$(EMCC) $(EMCC_CFLAGS) $(EMCC_LDFLAGS) -I build/mpg123/src/libmpg123 -I mpg123/src/libmpg123 $< build/mpg123/src/libmpg123/.libs/libmpg123.a -s RESERVED_FUNCTION_POINTERS=50 -s EXPORTED_FUNCTIONS="['_Mpg123Initialize','_Mpg123Decode','_Mpg123Destroy']" -o $@
