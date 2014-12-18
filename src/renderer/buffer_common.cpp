#include "buffer_common.h"
#include "enums.h"

namespace {
using namespace std;

void tidy_buffer(vector<uint8_t>& data, vector<BufferRange>& del_ranges){
//XXX: handle overlapping ranges correctly
	for(auto& r : del_ranges){
		data.erase(data.begin() + r.off, data.begin() + r.off + r.len);
		for(auto& r2 : del_ranges){
			if(r2.off > r.off) r2.off -= r.len;
		}

		if(r.callback){
			r.callback->onBufferRangeInvalidated(r.off, r.len);
		}
	}
	del_ranges.clear();
}

}

StreamingBuffer::StreamingBuffer()
: data(nullptr)
, id(0)
, type(0)
, prev_size(0)
, prev_capacity(0)
, dirty(false) {

}

StreamingBuffer::StreamingBuffer(GLenum type, std::vector<uint8_t>& buff)
: data(&buff)
, id(0)
, type(type)
, prev_size(0)
, prev_capacity(0)
, dirty(true) {
	gl.GenBuffers(1, &id);
	gl.BindBuffer(type, id);
	gl.BufferData(type, data->capacity(), nullptr, GL_STREAM_DRAW);
}

void StreamingBuffer::mark(){
	dirty = true;
}

void StreamingBuffer::invalidate(BufferRange&& range){
	unused_ranges.push_back(std::move(range));
}

void StreamingBuffer::update(){
/* TODO: more efficient buffer streaming
	https://www.opengl.org/wiki/Buffer_Object_Streaming
	http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
*/
	if(!dirty) return;

	gl.BindBuffer(type, id);
	
	if(gl.streaming_mode->get() == BUFFER_INVALIDATE){
	
		if(gl.InvalidateBufferData){
			gl.InvalidateBufferData(id);
		} else {
			log(logging::warn, "Streaming mode is BUFFER_INVALIDATE, but glInvalidateBufferData unsupported!");
		}
	
		tidy_buffer(*data, unused_ranges);

		if(prev_capacity != data->capacity()){
			gl.BufferData(type, data->capacity(), data->data(), GL_STREAM_DRAW);
		} else {
			::printf("bsd: sz: %lu, cap: %lu\n", data->size(), data->capacity());
			gl.BufferSubData(type, 0, data->size(), data->data());
		}
	}
	
	if(gl.streaming_mode->get() == BUFFER_DATA_NULL){
		tidy_buffer(*data, unused_ranges);
		gl.BufferData(type, data->capacity(), nullptr, GL_STREAM_DRAW);
		gl.BufferSubData(type, 0, data->size(), data->data());
	}
	
	if(gl.streaming_mode->get() == MAP_INVALIDATE){
		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
		
		if(prev_capacity != data->capacity()){
			gl.BufferData(type, data->capacity(), nullptr, GL_STREAM_DRAW);
		} else {
			flags |= GL_MAP_INVALIDATE_BUFFER_BIT;
		}

		tidy_buffer(*data, unused_ranges);
		
		void* gl_data = gl.MapBufferRange(
			type, 
			0, 
			data->size(),
			flags
		);
		
		memcpy(gl_data, data->data(), data->size());
		
		gl.UnmapBuffer(type);
	}
	
	if(gl.streaming_mode->get() == MAP_UNSYNC_APPEND){
		if(prev_capacity != data->capacity()){
			tidy_buffer(*data, unused_ranges);
			gl.BufferData(type, data->capacity(), nullptr, GL_STREAM_DRAW);
			prev_size = 0;
		}
	
		void* gl_data = gl.MapBufferRange(
			type,
			prev_size,
			data->size() - prev_size,
			GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT
		);
		
		memcpy(gl_data, data->data() + prev_size, data->size() - prev_size);
		
		gl.UnmapBuffer(type);
	}
	
	if(gl.streaming_mode->get() == DOUBLE_BUFFER){
		log(logging::error, "gl_streaming_mode DOUBLE_BUFFER NYI");
	}

	dirty = false;
	prev_capacity = data->capacity();
	prev_size = data->size();

}

StreamingBuffer::~StreamingBuffer(){
	if(id && gl.initialized()){
		gl.DeleteBuffers(1, &id);
	}
}


