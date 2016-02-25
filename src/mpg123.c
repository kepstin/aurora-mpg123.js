#include <mpg123.h>

static int initialized = 0;

typedef struct {
	mpg123_handle *handle;
	float *outbuf;
	int outlen;
	int read_format;
} Mpg123;

/* Callback to notify how many samples have been decoded */
typedef void (*AVDataCallback)(int samples);

/* Callback to notify on format information available */
typedef void (*AVFormatCallback)(int rate, int channels, int enc);

/* Create a library handle, and prepare it for use in a decoder
 * Returns NULL on error. */
Mpg123 *Mpg123Initialize(float *outbuf, int outlen) {
	if (!initialized) {
		mpg123_init();
		initialized = 1;
	}

	Mpg123 *mpg123 = calloc(1, sizeof(Mpg123));
	mpg123->handle = mpg123_new(NULL, NULL);
	mpg123_open_feed(mpeg123->handle);
}

/* Decode some audio data. Feeds the provided data into the decoder,
 * then calls the format and data callbacks depending on the result
 * of the decoder.
 * Note that the data callback can be called 0 or more times. The
 * output buffer might be re-used immediately after the output callback
 * returns.
 * Returns 0 on success. The return value is an mpg123_error enum value. */
int Mpg123Decode(Mpg123 *mpg123, void *inbuf, int inbuflen,
		AVDataCallback data_cb, AVFormatCallback format_cb) {
	int ret;
	/* Start by feeding newly provided data into the decoder */
	ret = mpg123_feed(mpg123->handle, inbuf, inbuflen);
	if (ret != MPG123_OK)
		return ret;
	return 0;
}
