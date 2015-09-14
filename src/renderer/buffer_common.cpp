#include "buffer_common.h"
#include "enums.h"
#include "render_state.h"
#include "cvar.h"
#include <algorithm>

namespace {
using namespace std;

static bool biggest_off_first(const BufferRange& a, const BufferRange& b){
	return a.off > b.off;
}

static void tidy_buffer(vector<uint8_t>& data, vector<BufferRange>& del_ranges, size_t& unused_bytes){
	//XXX: handle overlapping ranges correctly

	sort(del_ranges.begin(), del_ranges.end(), &biggest_off_first);

	for(auto& r : del_ranges){
		data.erase(data.begin() + r.off, data.begin() + r.off + r.len);

		if(r.callback){
			r.callback->onBufferRangeInvalidated(r.off, r.len);
		}
	}

	del_ranges.clear();
	unused_bytes = 0;
}

}

StreamingBuffer::StreamingBuffer()
: data(nullptr)
, id(0)
, type(0)
, prev_size(0)
, prev_capacity(0)
, dirty(false)
, no_async(false) {

}

StreamingBuffer::StreamingBuffer(GLenum type, std::vector<uint8_t>& buff, bool append_only)
: data(&buff)
, id(0)
, type(type)
, prev_size(0)
, prev_capacity(data->capacity())
, dirty(buff.size() != 0)
, no_async(!append_only) {
	gl.GenBuffers(1, &id);
	gl.BindBuffer(type, id);
	if(prev_capacity) gl.BufferData(type, prev_capacity, nullptr, GL_STREAM_DRAW);
}

void StreamingBuffer::mark(){
	dirty = true;
}

void StreamingBuffer::invalidate(BufferRange&& range){
	unused_bytes += range.len;
	unused_ranges.push_back(std::move(range));
}

void StreamingBuffer::invalidateAll(){
	prev_size = 0;
	dirty = true;
}

void StreamingBuffer::update(RenderState& rs){
/* TODO: more efficient buffer streaming
	https://www.opengl.org/wiki/Buffer_Object_Streaming
	http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
*/
	if(!dirty) return;
	
	GLuint* rs_buffer = nullptr;

	switch(type){
		case GL_ELEMENT_ARRAY_BUFFER: rs_buffer = &rs.ibo; break;
		case GL_ARRAY_BUFFER:         rs_buffer = &rs.vbo; break;
	};

	if(rs_buffer && *rs_buffer != id){
		gl.BindBuffer(type, id);
		*rs_buffer = id;
	}

	bool done = false;
	
	if(gl.streaming_mode->get() == DOUBLE_BUFFER){
		log(logging::error, "gl_streaming_mode DOUBLE_BUFFER NYI");
		gl.streaming_mode->set(BUFFER_INVALIDATE);
	}
	
	if(gl.streaming_mode->get() == MAP_INVALIDATE && !no_async){
		
		if(!gl.MapBufferRange || !gl.UnmapBuffer){
			log(logging::warn, "glMapBufferRange unavailable, using BUFFER_DATA_NULL.");
			gl.streaming_mode->set(BUFFER_DATA_NULL);
		} else {
			GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

			if(prev_capacity != data->capacity()){
				gl.BufferData(type, data->capacity(), nullptr, GL_STREAM_DRAW);
			} else {
				flags |= GL_MAP_INVALIDATE_BUFFER_BIT;
			}

			tidy_buffer(*data, unused_ranges, unused_bytes);
			
			void* gl_data = gl.MapBufferRange(
				type, 
				0, 
				data->size(),
				flags
			);
			
			memcpy(gl_data, data->data(), data->size());
			
			gl.UnmapBuffer(type);
		}
	}
	
	if(gl.streaming_mode->get() == MAP_UNSYNC_APPEND && !no_async){

		if(!gl.MapBufferRange || !gl.UnmapBuffer){
			log(logging::warn, "glMapBufferRange unavailable, using BUFFER_DATA_NULL.");
			gl.streaming_mode->set(BUFFER_DATA_NULL);
		} else {
			if(prev_capacity != data->capacity()
			|| unused_bytes/3 >= data->capacity()/4){
				tidy_buffer(*data, unused_ranges, unused_bytes);
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
	}
	
	if(gl.streaming_mode->get() == BUFFER_INVALIDATE || (no_async && !done)){
	
		if(gl.InvalidateBufferData){
			gl.InvalidateBufferData(id);	
			tidy_buffer(*data, unused_ranges, unused_bytes);

			if(prev_capacity != data->capacity()){
				gl.BufferData(type, data->capacity(), data->data(), GL_STREAM_DRAW);
			} else {
				gl.BufferSubData(type, 0, data->size(), data->data());
			}
			done = true;

		} else if(!no_async) {
			log(logging::warn, "glInvalidateBufferData unavailable, using BUFFER_DATA_NULL.");
			gl.streaming_mode->set(BUFFER_DATA_NULL);
		}
	}
	
	if(gl.streaming_mode->get() == BUFFER_DATA_NULL || (no_async && !done)){
		tidy_buffer(*data, unused_ranges, unused_bytes);
		gl.BufferData(type, data->capacity(), nullptr, GL_STREAM_DRAW);
		gl.BufferSubData(type, 0, data->size(), data->data());
		done = true;
	}
	dirty = false;
	prev_capacity = data->capacity();
	prev_size = data->size();
}

void StreamingBuffer::onGLContextRecreate(){
	GLuint new_id;
	gl.GenBuffers(1, &new_id);
	DEBUGF("Reloading streaming_buf. id [%u] -> [%u].", id, new_id);

	prev_capacity = 0;
	prev_size = 0;
	dirty = true;
	id = new_id;
}

StreamingBuffer::~StreamingBuffer(){
	if(id && gl.initialized()){
		gl.DeleteBuffers(1, &id);
	}
}


