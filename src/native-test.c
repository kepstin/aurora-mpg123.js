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
