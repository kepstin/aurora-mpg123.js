var AV = require('av');
var Mpg123 = require('../build/libmpg123.js');

var Mp3Decoder = AV.Decoder.extend(function() {
	AV.Decoder.register("mp3", this);

	this.prototype.init = function() {
		this.inlen = 4096; /* bytes */
		this.inbuf = Mpg123._malloc(this.inlen);
		this.outlen = 4096; /* samples */
		this.outbuf = Mpg123._malloc(this.outlen << 2);

		this.format_cb = Mpg123.Runtime.addFunction(function(rate, channels) {
			if (this.format.sampleRate != rate ||
					this.format.channelsPerFrame != channels) {
				throw new TypeError("Rate or Channels mismatch in MP3 stream");
			}
		}.bind(this));


	};
});

module.exports = Mp3Decoder;
