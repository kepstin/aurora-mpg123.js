var AV = require('av');

var MP3Demuxer = AV.Demuxer.extend(function() {
	AV.Demuxer.register(this);

	this.probe = function(stream) {
		// The MP3 probing is mostly ad-hoc matching. You don't really
		// know if something is MP3 until you (fail to) decode it.
		var off = stream.offset;

		// Check for an ID3v2 header at the start of the file
		var id3 = stream.readUint24(false);
		if (id3 == 0x494433) {
			// Matches the string 'ID3'
			stream.advance(3);
			var size = 0;
			size |= stream.readUint8() << 21;
			size |= stream.readUint8() << 14;
			size |= stream.readUint8() << 7;
			size |= stream.readUint8() << 0;

			// Skip over the header
			stream.advance(size);
		} else {
			// No ID3, go back and look for sync.
			stream.seek(off);
		}

		// Check for sync
		var sync = stream.readUInt16(false);
		if (sync & 0xFFFE != 0xFFFA) {
			stream.seek(off);
			return false;
		}

		// Check for a valid bitrate
		var rate = stream.readUint8() & 0xF0;
		if (rate < 0x10 || rate > 0xE0) {
			stream.seek(off);
			return false;
		}

		// It's... probably an MP3 (MPEG-1 Layer 3), I guess?
		return true;
	};

	this.prototype.readChunk = function() {
		var stream = this.stream;

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
