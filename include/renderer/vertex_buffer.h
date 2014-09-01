#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_
#include "gl_context.h"

struct VertexBuffer {
	virtual void applyFormat(int attr, int buffer_index) = 0;
	virtual GLuint getID(void) = 0;
};

// e.g. "1bI:2BN:2s:4S:4fN" caps = unsigned

struct StaticVertexBuffer : VertexBuffer {
	StaticVertexBuffer(const char* fmt, Buffer& data){
		gl.GenBuffers(1, &id);
		gl.BindBuffer(GL_ARRAY_BUFFER, id);
		gl.BufferData(GL_ARRAY_BUFFER, data.size, data.data, GL_STATIC_DRAW);
	}
	virtual void applyFormat(int attr, int buffer_index){
		size_t i = buffer_index;
		assert(i < attrs.size());
		
		if(attrs[i].integral){
			gl.VertexAttribIFormat(attr, attrs[i].size, attrs[i].type, attrs[i].off);
		} else {
			gl.VertexAttribFormat(attr, attrs[i].size, attrs[i].type, attrs[i].normalised, attrs[i].off);
		}
	}
	virtual void invalidate(void){
		
	}
	virtual GLuint getID(void){
		return id;
	}
private:
	struct attr {
		int size, off;
		GLenum type;
		bool normalised, integral;
	};
	std::vector<attr> attrs;
	GLuint id;
	size_t size;
};

struct DynamicVertexBuffer : VertexBuffer {

	void applyFormat(int attr, int buffer_index){
	
	}
	GLuint getID(void){
		return 0;
	}

};

#endif
