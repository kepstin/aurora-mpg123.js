/* @license
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

var AV = require('av');
var Mpg123 = require('../build/libmpg123.js');

var MP3Decoder = AV.Decoder.extend(function() {
	AV.Decoder.register('mp3', this);

	this.prototype.init = function() {
		this.inlen = 8192; /* bytes */
		this.inbuf = Mpg123._malloc(this.inlen);
		this.outlen = 65536; /* samples */
		this.outbuf = Mpg123._malloc(this.outlen << 2);
		this.outchunks = [];
		this.outchunkslen = 0;

		this.mpg123 = Mpg123._Mpg123Initialize(this.outbuf, this.outlen);

		var self = this;

		this.format_cb = Mpg123.Runtime.addFunction(function(rate, channels) {
			self.checkFormat(rate, channels);
		});

		this.data_cb = Mpg123.Runtime.addFunction(function(data) {
			self.emitData(data);
		});
	}

	this.prototype.destroy = function() {
		Mpg123._free(this.inbuf);
		this.inbuf = null;
		Mpg123._free(this.outbuf);
		this.outbuf = null
		Mpg123._Mpg123Destroy(this.mpg123);
		this.mpg123 = null;
		Mpg123.removeFunction(this.format_cb);
		Mpg123.removeFunction(this.data_cb);

	}

	this.prototype.decode = function() {
		this.waiting = !this.receivedFinalBuffer;
		var offset = this.bitstream.offset;

		try {
			this.readChunk();
		} catch (error) {
			if (!(error instanceof AV.UnderflowError)) {
				this.emit('error', error);
				return false;
			}
		}

		if (this.outchunkslen > 0) {
			var data = new Float32Array(this.outchunkslen);
			var offset = 0;
			for (var i = 0; i < this.outchunks.length; i++) {
				data.set(this.outchunks[i], offset);
				offset += this.outchunks[i].length;
			}
			this.outchunks = [];
			this.outchunkslen = 0;

			this.emit('data', data);
			if (this.receivedFinalBuffer)
				this.emit('end');
			return true
		} else if (!this.receivedFinalBuffer) {
			this.waiting = true;
		} else {
			this.emit('end');
		}

		return false;
	}

	this.prototype.readChunk = function() {
		if (!this.stream.available(1))
			throw new AV.UnderflowError();

		var list = this.stream.list;
		var packet = list.first;
		list.advance();

		var data = packet.data;

		var decoded = 0;

		while (decoded < data.length) {
			var inlen = Math.min(this.inlen, data.length - decoded);
			var slice = data.subarray(decoded, decoded + inlen);
			Mpg123.HEAPU8.set(slice, this.inbuf);

			var ret = Mpg123._Mpg123Decode(this.mpg123,
					this.inbuf, inlen,
					this.data_cb, this.format_cb);

			if (ret != 0) {
				throw new Error("Mpg123 decoding error: " + ret);
			}

			decoded += inlen;
		}
	}

	this.prototype.checkFormat = function(rate, channels) {
		if (rate != this.format.sampleRate || channels != this.format.channelsPerFrame)
			throw new TypeError("Sample Rate or Channels changed: Expected " + this.format.sampleRate + "/" + this.format.channelsPerFrame + ", got " + rate + "/" + channels);
	}

	this.prototype.emitData = function(len) {
		var samples = Mpg123.HEAPF32.subarray(this.outbuf >> 2,
				(this.outbuf >> 2) + len);
		this.outchunks.push(new Float32Array(samples));
		this.outchunkslen += samples.length;
	}
});

module.exports = MP3Decoder;
