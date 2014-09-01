#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "resource_system.h"

struct IndexBuffer {
	IndexBuffer(GLenum type, std::shared_ptr<Buffer>& data){
		gl.GenBuffers(1, &id);
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, data->size, data->data, GL_STREAM_DRAW);
	}
	
	void bind(void){
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}
	
//private:
	GLuint id;
	GLenum type;
};

#endif
