.PHONY: all
all: mpg123

.PHONY: clean
clean:
	rm -rf build

.PHONY: mpg123
mpg123: build/Makefile
	$(MAKE) -C build

build/Makefile:
	mkdir -p build
	(cd build && emconfigure ../mpg123/configure --with-cpu=generic_fpu --disable-fifo --disable-network --disable-layer1 --disable-layer2 --disable-shared --enable-static --disable-ntom --disable-downsample --disable-icy --disable-messages --disable-feature-report --disable-8bit --disable-16bit --disable-32bit --disable-equalizer)

