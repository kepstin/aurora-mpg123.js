var AV = require('av');

var MP3Demuxer = AV.Demuxer.extend(function() {
	AV.Demuxer.register(this);

	this.getFormatInfo = this.prototype.getFormatInfo = function(stream) {
		var off = stream.offset;

		// Check for an ID3v2 header at the start of the file
		var id3 = stream.readUInt24(false);
		if (id3 == 0x494433) {
			// Matches the string 'ID3'
			stream.advance(3);
			var size = 0;
			size |= stream.readUInt8() << 21;
			size |= stream.readUInt8() << 14;
			size |= stream.readUInt8() << 7;
			size |= stream.readUInt8() << 0;

			// Skip over the header
			stream.advance(size);
		} else {
			// No ID3, go back and look for sync.
			stream.seek(off);
		}

		// Check for sync
		var sync = stream.readUInt16(false);
		if ((sync & 0xFFFE) != 0xFFFA) {
			stream.seek(off);
			return null;
		}

		var rate = stream.readUInt8();

		// Check for a valid bitrate
		if ((rate & 0xF0) < 0x10 || (rate & 0xF0) > 0xE0) {
			stream.seek(off);
			return null;
		}

		// Determine the sample rate
		var sr = 0;
		if ((rate & 0x0C) == 0x00)
			sr = 44100;
		else if ((rate & 0x0C) == 0x04)
			sr = 48000;
		else if ((rate & 0x0C) == 0x08)
			sr = 32000;
		else {
			stream.seek(off);
			return null;
		}

		stream.seek(off);

		return {
			formatID: "mp3",
			sampleRate: sr,
			channelsPerFrame: 2, /* Decoder hardcoded to output 2 channels */
			floatingPoint: true
		};
	}

	this.probe = function(stream) {
		// The MP3 probing is mostly ad-hoc matching. You don't really
		// know if something is MP3 until you (fail to) decode it.

		var info = this.getFormatInfo(stream);

		if (info != null) {
			// It's... probably an MP3 (MPEG-1 Layer 3), I guess?
			return true;
		} else {
			return false;
		}
	};

	this.prototype.readChunk = function() {
		var stream = this.stream;

		// Need to parse enough of the file to get format & samplerate
		// so that the decoder can be selected (and used)
		if (!this.sentInfo) {
			this.emit("format", this.getFormatInfo(stream));
			this.sentInfo = true;
		}

		// MP3 isn't really muxed; just hand the data straight to
		// the decoder.
		while (stream.available(1)) {
			var buffer = stream.readSingleBuffer(
					stream.remainingBytes());
			this.emit('data', buffer);
		}
	};
});

module.exports = MP3Demuxer;
AV.MP3Demuxer = MP3Demuxer;
