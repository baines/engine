#include "index_buffer.h"

StaticIndexBuffer::StaticIndexBuffer()
: data()
, type(0)
, id(0) {

}

StaticIndexBuffer::StaticIndexBuffer(const ResourceHandle& data, GLenum type)
: data(data)
, type(type)
, id(0) {
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
}

void StaticIndexBuffer::bind(void){
	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

GLenum StaticIndexBuffer::getType() const {
	return type;
}

GLenum StaticIndexBuffer::getID() const {
	return id;
}

StaticIndexBuffer::~StaticIndexBuffer(){
	gl.DeleteBuffers(1, &id);
}

