PKG_CONFIG := pkg-config
EMCC := emcc
EMCC_CFLAGS = -O3

.PHONY: all
all: mpg123

.PHONY: clean
clean:
	rm -rf build

.PHONY: mpg123
mpg123: build/Makefile
	$(MAKE) -C build

.PHONY: native-test
native-test: build/native-test

build/Makefile:
	mkdir -p build
	(cd build && emconfigure ../mpg123/configure --with-cpu=generic_fpu --disable-fifo --disable-network --disable-layer1 --disable-layer2 --disable-shared --enable-static --disable-ntom --disable-downsample --disable-icy --disable-messages --disable-feature-report --disable-8bit --disable-16bit --disable-32bit --disable-equalizer)

build/native-test: src/mpg123.c src/native-test.c
	mkdir -p build
	$(CC) $^ `$(PKG_CONFIG) --cflags --libs libmpg123` -o $@
