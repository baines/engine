#ifndef BUFFER_COMMON_H
#define BUFFER_COMMON_H
#include "common.h"
#include "gl_context.h"
#include <vector>

struct StreamingBuffer {

	StreamingBuffer();
	StreamingBuffer(GLenum type, std::vector<uint8_t>& buff);
	void mark();
	void update();
	GLuint getID() const {
		return id;
	}
	~StreamingBuffer();

private:
	std::vector<uint8_t>* data;
	GLuint id;
	GLenum type;
	size_t prev_size, prev_capacity;
	bool dirty;
};

#endif
