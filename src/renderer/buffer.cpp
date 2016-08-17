#include "buffer_common.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "enums.h"
#include "cvar.h"
#include <algorithm>
#include <numeric>

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
	if(prev_capacity){
		gl.BindBuffer(type, id);
		gl.BufferData(type, prev_capacity, nullptr, GL_STREAM_DRAW);
	}
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

void StreamingBuffer::update(){
/* TODO: more efficient buffer streaming
	https://www.opengl.org/wiki/Buffer_Object_Streaming
	http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
*/
	if(!dirty) return;
	
	GLuint* buffer = nullptr;

	switch(type){
		case GL_ELEMENT_ARRAY_BUFFER: buffer = &gl.state.ibo; break;
		case GL_ARRAY_BUFFER:         buffer = &gl.state.vbo; break;
	};

	if(buffer && *buffer != id){
		gl.BindBuffer(type, id);
		*buffer = id;
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
	
	if(gl.streaming_mode->get() == BUFFER_INVALIDATE && (no_async || !done)){
	
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



namespace {

static GLenum lookup_type(const char c){
	switch(c){
		case 'b': return GL_BYTE;
		case 'B': return GL_UNSIGNED_BYTE;
		case 's': return GL_SHORT;
		case 'S': return GL_UNSIGNED_SHORT;
		case 'i': return GL_INT;
		case 'I': return GL_UNSIGNED_INT;
		case 'f': return GL_FLOAT;
	/*	case 'F': return GL_FIXED; */
		case 'r': return GL_INT_2_10_10_10_REV;
		case 'R': return GL_UNSIGNED_INT_2_10_10_10_REV;
		case 'h': return GL_HALF_FLOAT;
		case 'd': return GL_DOUBLE;
		default:  return 0;
	}
}

static int lookup_size(GLenum e){
	switch(e){
		case GL_BYTE:
		case GL_UNSIGNED_BYTE: 
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_HALF_FLOAT:
			return 2;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
		case GL_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			return 4;
		case GL_DOUBLE:
			return 8;
		default:
			return 0;
	}
}

// format should be (<name>':'<nelem><type>'|')+ e.g. "a_pos:4S|a_tex:2f"

static void parse_attribs(const char* fmt, ShaderAttribs& attrs, GLint& stride) {

	enum { GET_NAME, GET_NELEM, GET_TYPE, GET_EXTRA } state = GET_NAME;
	
	ShaderAttribs::Attrib current_attr{ };
	stride = 0;
	
	DEBUGF("Starting to parse VBO format...");
	
	auto add_attr = [&](ShaderAttribs::Attrib& at){
		if(at.nelem != 0 && at.type != 0){
			int aligned_sz = (
				at.nelem
				* lookup_size(at.type) 
				+ 3
			) & ~3;
	
			at.off = stride;
			stride += aligned_sz;
		
			attrs.setAttribFormat(at.name_hash, at.type, at.nelem, at.off, at.flags);
			DEBUGF("---- Attrib added.");
		}
	};
	
	const char* name_start = fmt;
	
	for(const char* p = fmt; *p; ++p){
		if(*p == '|'){
			add_attr(current_attr);
			current_attr = ShaderAttribs::Attrib();
			name_start = p + 1;
			state = GET_NAME;
		} else {
			if(state == GET_NAME && *p == ':'){
				const int len = p - name_start;
				DEBUGF("\tAttrib name: %.*s", len, name_start);
				current_attr.name_hash = str_hash_len(name_start, len);
				state = GET_NELEM;
			} else
			if(state == GET_NELEM){
				const int n = *p - 48;
				if(n > 0 && n <= 4){
					current_attr.nelem = n;
					DEBUGF("\tAttrib no. elements: %d", n);
					state = GET_TYPE;
				}
			} else
			if(state == GET_TYPE){
				if(GLenum t = lookup_type(*p)){
					current_attr.type = t;
					DEBUGF("\tAttrib type: %#x", t);
					state = GET_EXTRA;
				}
			} else
			if(state == GET_EXTRA){
				if(*p == 'N') current_attr.flags |= ATR_NORM;
				if(*p == 'I') current_attr.flags |= ATR_INT;
				if(*p == '/') current_attr.flags |= ATR_INSTANCED;
				DEBUGF("\tAttrib flags: %#x", current_attr.flags);
			}
		}
	}
	add_attr(current_attr);
}

}

void VertexBuffer::bind(){
	auto id = getID();
	if(id && gl.state.vbo != id){
		gl.BindBuffer(GL_ARRAY_BUFFER, id);
		gl.state.vbo = id;
	}
}

StaticVertexBuffer::StaticVertexBuffer()
: data()
, attrs()
, stride(0)
, id(0) {

}

StaticVertexBuffer::StaticVertexBuffer(const MemBlock& data, const char* fmt)
: data(data)
, attrs()
, stride(0)
, id(0) {
	parse_attribs(fmt, attrs, stride);
	
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ARRAY_BUFFER, id);
	gl.BufferData(GL_ARRAY_BUFFER, data.size, data.ptr, GL_STATIC_DRAW);
	gl.state.vbo = id;
}

const ShaderAttribs& StaticVertexBuffer::getShaderAttribs(void) const {
	return attrs;
}

GLint StaticVertexBuffer::getStride(void) const {
	return stride;
}

size_t StaticVertexBuffer::getSize(void) const {
	return stride ? data.size / stride : 0;
}

GLuint StaticVertexBuffer::getID(void) const {
	return id;
}

void StaticVertexBuffer::update(){

}

void StaticVertexBuffer::onGLContextRecreate() {
	GLuint new_id;
	gl.GenBuffers(1, &new_id);
	DEBUGF("Reloading static vbo: [%d] -> [%d].", id, new_id);
	id = new_id;
	gl.BindBuffer(GL_ARRAY_BUFFER, id);
	gl.BufferData(GL_ARRAY_BUFFER, data.size, data.ptr, GL_STATIC_DRAW);
	gl.state.vbo = id;
}

StaticVertexBuffer::~StaticVertexBuffer(){
	if(id && gl.initialized()){
		gl.DeleteBuffers(1, &id);
	}
}

DynamicVertexBuffer::DynamicVertexBuffer()
: data()
, attrs()
, stride(0)
, stream_buf() {

}

DynamicVertexBuffer::DynamicVertexBuffer(const char* fmt, size_t initial_capacity)
: data()
, attrs()
, stride(0)
, stream_buf(GL_ARRAY_BUFFER, data, true) {
	parse_attribs(fmt, attrs, stride);
	data.reserve(initial_capacity);	
}

void DynamicVertexBuffer::clear(){
	data.clear();
	stream_buf.mark();
}

uint8_t* DynamicVertexBuffer::beginWrite(size_t upper_bound){
	stream_buf.invalidateAll();
	data.clear();
	data.resize(upper_bound);
	return data.data();
}

void DynamicVertexBuffer::endWrite(size_t bytes_written){
	data.resize(bytes_written);
	data.shrink_to_fit();
}

void DynamicVertexBuffer::invalidate(BufferRange&& range){
	stream_buf.invalidate(std::move(range));
}

void DynamicVertexBuffer::onGLContextRecreate(){
	gl.validateObject(stream_buf);
}

const ShaderAttribs& DynamicVertexBuffer::getShaderAttribs() const {
	return attrs;
}

GLint DynamicVertexBuffer::getStride() const {
	return stride;
}

size_t DynamicVertexBuffer::getSize() const {
	return stride ? data.size() / stride : 0;
}

GLuint DynamicVertexBuffer::getID() const {
	return stream_buf.getID();
}

void DynamicVertexBuffer::update(){
	stream_buf.update();
}

StaticIndexBuffer::StaticIndexBuffer()
: data()
, type(0)
, id(0) {

}

StaticIndexBuffer::StaticIndexBuffer(const MemBlock& data, GLenum type)
: data(data)
, type(type)
, id(0) {
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, data.size, data.ptr, GL_STATIC_DRAW);
	gl.state.vbo = id;
}

void StaticIndexBuffer::bind(){
	if(gl.state.ibo != id){
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		gl.state.ibo = id;
	}
}

GLenum StaticIndexBuffer::getType() const {
	return type;
}

GLenum StaticIndexBuffer::getID() const {
	return id;
}

void StaticIndexBuffer::onGLContextRecreate(){
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, data.size, data.ptr, GL_STATIC_DRAW);
	gl.state.ibo = id;
}

StaticIndexBuffer::~StaticIndexBuffer(){
	gl.DeleteBuffers(1, &id);
}

template<class T> struct index_type {};
template<> struct index_type<uint8_t>{ static const GLenum value = GL_UNSIGNED_BYTE; };
template<> struct index_type<uint16_t>{ static const GLenum value = GL_UNSIGNED_SHORT; };
template<> struct index_type<uint32_t>{ static const GLenum value = GL_UNSIGNED_INT; };

template<class T>
DynamicIndexBuffer<T>::DynamicIndexBuffer()
: indices()
, stream_buf(GL_ELEMENT_ARRAY_BUFFER, indices, false){

}
template<class T>
void DynamicIndexBuffer<T>::replace(size_t index, T val){
	if((index + 1) * sizeof(T) > indices.size()) return;

	auto* p = reinterpret_cast<const uint8_t*>(&val);
	for(size_t i = 0; i < sizeof(T); ++i){
		indices[index * sizeof(T) + i] = p[i];
	}
	stream_buf.invalidateAll();
}

template<class T>
void DynamicIndexBuffer<T>::push(T val){
	auto* p = reinterpret_cast<const uint8_t*>(&val);
	for(size_t i = 0; i < sizeof(T); ++i){
		indices.push_back(p[i]);
	}
	stream_buf.mark();
}

template<class T>
void DynamicIndexBuffer<T>::clear(){
	indices.clear();
	stream_buf.mark();
}

template<class T>
uint8_t* DynamicIndexBuffer<T>::beginWrite(size_t upper_bound){
	stream_buf.invalidateAll();
	indices.clear();
	indices.resize(upper_bound);
	return indices.data();
}

template<class T>
void DynamicIndexBuffer<T>::endWrite(size_t bytes_written){
	indices.resize(bytes_written);
	indices.shrink_to_fit();
}

template<class T>
GLenum DynamicIndexBuffer<T>::getType() const {
	return index_type<T>::value;
}

template<class T>
GLuint DynamicIndexBuffer<T>::getID() const {
	return stream_buf.getID();
}

template<class T>
size_t DynamicIndexBuffer<T>::getSize() const {
	return indices.size() / sizeof(T);
}

template<class T>
void DynamicIndexBuffer<T>::bind(){
	auto id = stream_buf.getID();
	if(gl.state.ibo != id){
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		gl.state.ibo = id;
	}
}

template<class T>
void DynamicIndexBuffer<T>::update(){
	stream_buf.update();
}

template<class T>
void DynamicIndexBuffer<T>::onGLContextRecreate(){
	gl.validateObject(stream_buf);
}

template struct DynamicIndexBuffer<uint8_t>;
template struct DynamicIndexBuffer<uint16_t>;
template struct DynamicIndexBuffer<uint32_t>;
