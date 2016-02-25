#include <mpg123.h>

static int lib_initialized = 0;

typedef struct {
	mpg123_handle *handle;
	float *outbuf;
	int outlen;
	int read_format;
} Mpg123;

/* Callback to notify how many samples have been decoded */
typedef void (*AVDataCallback)(int samples);

/* Callback to notify on format information available */
typedef void (*AVFormatCallback)(int rate, int channels);

/* Create a library handle, and prepare it for use in a decoder
 * Returns NULL on error. */
Mpg123 *Mpg123Initialize(float *outbuf, int outlen) {
	int ret;
	if (!lib_initialized) {
		ret = mpg123_init();
		if (ret != MPG123_OK) return NULL;
		lib_initialized = 1;
	}

	Mpg123 *mpg123 = calloc(1, sizeof(Mpg123));

	mpg123->outbuf = outbuf;
	mpg123->outlen = outlen * sizeof(float);

	mpg123->handle = mpg123_new(NULL, &ret);
	if (ret != MPG123_OK) goto err_free;

	/* Set up the acceptable output formats.
	 * We only want float32, but mono/stereo and any sample rate is ok.
	 * Only enabling the sample rates supported by MPEG-1 layer 3. */
	ret = mpg123_format_none(mpg123->handle);
	if (ret != MPG123_OK) goto err_free;
	ret = mpg123_format(mpg123->handle, 32000, MPG123_STEREO|MPG123_MONO, MPG123_ENC_FLOAT_32);
	if (ret != MPG123_OK) goto err_free;
	ret = mpg123_format(mpg123->handle, 44100, MPG123_STEREO|MPG123_MONO, MPG123_ENC_FLOAT_32);
	if (ret != MPG123_OK) goto err_free;
	ret = mpg123_format(mpg123->handle, 48000, MPG123_STEREO|MPG123_MONO, MPG123_ENC_FLOAT_32);

	mpg123_open_feed(mpg123->handle);
	if (ret != MPG123_OK) goto err_free;

	return mpg123;

err_free:
	free(mpg123);
	return NULL;
}

/* Decode some audio data. Feeds the provided data into the decoder,
 * then calls the format and data callbacks depending on the result
 * of the decoder.
 * Note that the data callback can be called 0 or more times. The
 * output buffer might be re-used immediately after the output callback
 * returns.
 * Returns 0 on success. The return value is an mpg123_error enum value. */
int Mpg123Decode(Mpg123 *mpg123, void *inbuf, int inlen,
		AVDataCallback data_cb, AVFormatCallback format_cb) {
	int ret;
	size_t done = 0;

	/* Feed in new input, then audio until the internal buffer drains */
	ret = mpg123_decode(mpg123->handle, inbuf, inlen, (unsigned char *) mpg123->outbuf, mpg123->outlen, &done);
	if (ret == MPG123_NEW_FORMAT) {
		long rate;
		int channels, enc;
		mpg123_getformat(mpg123->handle, &rate, &channels, &enc);
		format_cb(rate, channels);
	}
	if (done > 0)
		data_cb(done / sizeof(float));

	while (ret != MPG123_ERR && ret != MPG123_NEED_MORE) {
		ret = mpg123_decode(mpg123->handle, NULL, 0, (unsigned char *) mpg123->outbuf, mpg123->outlen, &done);
		if (done > 0)
			data_cb(done / sizeof(float));
	}
	
	if (ret == MPG123_OK || ret == MPG123_NEED_MORE) {
		return 0;
	} else {
		return ret;
	}
}

/* Close the stream and free the handle.
 * Does *not* deinitialize the decoder library; it might be re-used later,
 * and the JS code can just free the entire ASM.js block */
void Mpg123Destroy(Mpg123 *mpg123) {
	mpg123_close(mpg123->handle);
	mpg123_delete(mpg123->handle);
	free(mpg123);
}
