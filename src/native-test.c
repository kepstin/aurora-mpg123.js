/*
 * Copyright (c) 2016 Calvin Walton <calvin.walton@kepstin.ca>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Mpg123 Mpg123;
typedef void (*AVDataCallback)(int);
typedef void (*AVFormatCallback)(int, int);
Mpg123 *Mpg123Initialize(float *, int);
int Mpg123Decode(Mpg123 *, void *, int, AVDataCallback, AVFormatCallback);
void Mpg123Destroy(Mpg123 *);

#define BUF_LEN 1024

static void *inbuf;
static float *outbuf;

void data_cb(int samples) {
	fprintf(stderr, "AVDataCallback: %d samples\n", samples);
	fwrite(outbuf, sizeof(float), samples, stdout);
}

void format_cb(int rate, int channels) {
	fprintf(stderr, "AVFormatCallback: rate: %d, channels: %d\n", rate, channels);
}

int main(int argc, char *argv[]) {
	int ret;

	fprintf(stderr, "Pass an mp3 on stdin; raw audio will be given on stdout\n");

	inbuf = calloc(BUF_LEN, sizeof(uint8_t));
	outbuf = calloc(BUF_LEN, sizeof(float));

	fprintf(stderr, "Mpg123Initialize\n");
	struct Mpg123 *mpg123 = Mpg123Initialize(outbuf, BUF_LEN);
	if (mpg123 == NULL) {
		fprintf(stderr, "Mpeg123Initialize failed (returned NULL handle)\n");
		return 1;
	}
	fprintf(stderr, "Mpg123Initialize OK\n");

	int inbuflen;
	while (inbuflen = fread(inbuf, sizeof(uint8_t), BUF_LEN, stdin)) {
		fprintf(stderr, "Mpg123Decode: %d\n", inbuflen);
		ret = Mpg123Decode(mpg123, inbuf, inbuflen, data_cb, format_cb);
		if (ret != 0) {
			fprintf(stderr, "Error in Mpg123Decode: %d\n", ret);
			return 1;
		}
	}

	if (ferror(stdin)) {
		fprintf(stderr, "Error reading input\n");
		return 1;
	}

	return 0;
}
