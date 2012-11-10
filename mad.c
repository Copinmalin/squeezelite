/* 
 *  Squeezelite - lightweight headless squeezeplay emulator for linux
 *
 *  (c) Adrian Smith 2012, triode1@btinternet.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "squeezelite.h"

#include <mad.h>
#include <dlfcn.h>

#define READBUF_SIZE 2048 // local buffer used by decoder: FIXME merge with any other decoders needing one?

#define LIBMAD "libmad.so.0"

struct mad {
	u8_t *readbuf;
	unsigned readbuf_len;
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	// mad symbols to be dynamically loaded
	void (* mad_stream_init)(struct mad_stream *);
	void (* mad_frame_init)(struct mad_frame *);
	void (* mad_synth_init)(struct mad_synth *);
	void (* mad_frame_finish)(struct mad_frame *);
	void (* mad_stream_finish)(struct mad_stream *);
	void (* mad_stream_buffer)(struct mad_stream *, unsigned char const *, unsigned long);
	int  (* mad_frame_decode)(struct mad_frame *, struct mad_stream *);
	void (* mad_synth_frame)(struct mad_synth *, struct mad_frame const *);
	char const *(* mad_stream_errorstr)(struct mad_stream const *);
};

static struct mad *m;

extern log_level loglevel;

extern struct buffer *streambuf;
extern struct buffer *outputbuf;
extern struct streamstate stream;
extern struct outputstate output;
extern struct decodestate decode;

#define LOCK_S   pthread_mutex_lock(&streambuf->mutex)
#define UNLOCK_S pthread_mutex_unlock(&streambuf->mutex)
#define LOCK_O   pthread_mutex_lock(&outputbuf->mutex)
#define UNLOCK_O pthread_mutex_unlock(&outputbuf->mutex)

// based on libmad minimad.c scale
static inline u32_t scale(mad_fixed_t sample) {
	sample += (1L << (MAD_F_FRACBITS - 24));
	
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;
	
	return (s32_t)(sample >> (MAD_F_FRACBITS + 1 - 24)) << 8;
}

static void mad_decode(void) {
	LOCK_S;
	size_t bytes = min(_buf_used(streambuf), _buf_cont_read(streambuf));

	if (m->stream.next_frame && m->readbuf_len) {
		m->readbuf_len -= m->stream.next_frame - m->readbuf;
		memmove(m->readbuf, m->stream.next_frame, m->readbuf_len);
	}

	bytes = min(bytes, READBUF_SIZE - m->readbuf_len);
	memcpy(m->readbuf + m->readbuf_len, streambuf->readp, bytes);
	m->readbuf_len += bytes;
	_buf_inc_readp(streambuf, bytes);

	if (stream.state <= DISCONNECT && _buf_used(streambuf) == 0) {
		memset(m->readbuf + m->readbuf_len, 0, MAD_BUFFER_GUARD);
		m->readbuf_len += MAD_BUFFER_GUARD;
	}
	UNLOCK_S;

	m->mad_stream_buffer(&m->stream, m->readbuf, m->readbuf_len);

	while (true) {

		if (m->mad_frame_decode(&m->frame, &m->stream) == -1) {
			if (m->stream.error == MAD_ERROR_BUFLEN) {
				return;
			}
			if (!MAD_RECOVERABLE(m->stream.error)) {
				LOG_WARN("mad_frame_decode error: %s", m->mad_stream_errorstr(&m->stream));
				LOG_INFO("unrecoverable - stopping decoder");
				LOCK_O;
				decode.state = DECODE_COMPLETE;
				UNLOCK_O;
			} else {
				LOG_DEBUG("mad_frame_decode error: %s", m->mad_stream_errorstr(&m->stream));
			}
			return;
		};

		m->mad_synth_frame(&m->synth, &m->frame);

		LOCK_O;
		
		if (decode.new_stream) {
			LOG_INFO("setting track_start");
			output.next_sample_rate = m->synth.pcm.samplerate; 
			output.track_start = outputbuf->writep;
			decode.new_stream = false;
		}
		
		if (m->synth.pcm.length > _buf_space(outputbuf) / BYTES_PER_FRAME) {
			LOG_WARN("too many samples - dropping samples");
			m->synth.pcm.length = _buf_space(outputbuf) / BYTES_PER_FRAME;
		}
		
		size_t frames = m->synth.pcm.length;
		s32_t *iptrl = m->synth.pcm.samples[0];
		s32_t *iptrr = m->synth.pcm.samples[ m->synth.pcm.channels - 1 ];

		while (frames > 0) {
			size_t f = min(frames, _buf_cont_write(outputbuf) / BYTES_PER_FRAME);
			s32_t *optr = (s32_t *)outputbuf->writep;
			size_t count = f;
			while (count--) {
				*optr++ = scale(*iptrl++);
				*optr++ = scale(*iptrr++);
			}
			frames -= f;
			_buf_inc_writep(outputbuf, f * BYTES_PER_FRAME);
		}

		LOG_SDEBUG("write %u frames", m->synth.pcm.length);

		UNLOCK_O;
	}
}

static void mad_open(u8_t size, u8_t rate, u8_t chan, u8_t endianness) {
	m->readbuf_len = 0;
	m->mad_stream_init(&m->stream);
	m->mad_frame_init(&m->frame);
	m->mad_synth_init(&m->synth);
}

static void mad_close(void) {
	mad_synth_finish(&m->synth);
	m->mad_frame_finish(&m->frame);
	m->mad_stream_finish(&m->stream);
}

static bool load_mad() {
	void *handle = dlopen(LIBMAD, RTLD_NOW);
	if (!handle) {
		LOG_WARN("dlerror: %s", dlerror());
		return false;
	}

	m = malloc(sizeof(struct mad));
	m->readbuf = malloc(READBUF_SIZE + MAD_BUFFER_GUARD);

	m->readbuf_len = 0;
	m->mad_stream_init = dlsym(handle, "mad_stream_init");
	m->mad_frame_init = dlsym(handle, "mad_frame_init");
	m->mad_synth_init = dlsym(handle, "mad_synth_init");
	m->mad_frame_finish = dlsym(handle, "mad_frame_finish");
	m->mad_stream_finish = dlsym(handle, "mad_stream_finish");
	m->mad_stream_buffer = dlsym(handle, "mad_stream_buffer");
	m->mad_frame_decode = dlsym(handle, "mad_frame_decode");
	m->mad_synth_frame = dlsym(handle, "mad_synth_frame");
	m->mad_stream_errorstr = dlsym(handle, "mad_stream_errorstr");

	char *err;
	if ((err = dlerror()) != NULL) {
		LOG_WARN("dlerror: %s", err);		
		return false;
	}

	LOG_INFO("loaded "LIBMAD);
	return true;
}

struct codec *register_mad(void) {
	static struct codec ret = { 
		.id    = 'm',
		.types = "mp3",
		.open  = mad_open,
		.close = mad_close,
		.decode= mad_decode,
		.min_space = 204800,
		.min_read_bytes = READBUF_SIZE,
	};

	if (!load_mad()) {
		return NULL;
	}

	return &ret;
}
