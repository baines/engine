#ifndef BUFFER_COMMON_H
#define BUFFER_COMMON_H
#include "common.h"
#include "gl_context.h"
#include <vector>

struct BufferInvalidateListener {
	virtual void onBufferRangeInvalidated(size_t off, size_t len) = 0;
};

struct BufferRange {
	size_t off, len;
	BufferInvalidateListener* callback;
};

struct StreamingBuffer : public GLObject {

	StreamingBuffer();
	StreamingBuffer(GLenum type, std::vector<uint8_t>& buff);
	void mark();
	void invalidate(BufferRange&& range);
	void update();
	void onGLContextRecreate();
	GLuint getID() const {
		return id;
	}
	~StreamingBuffer();

private:
	std::vector<uint8_t>* data;
	std::vector<BufferRange> unused_ranges;
	GLuint id;
	GLenum type;
	size_t prev_size, prev_capacity, unused_bytes;
	bool dirty;
};

#endif
