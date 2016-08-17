#include "vertex_state.h"
#include "vertex_buffer.h"
#include "index_buffer.h"

static void bind_vao(VertexState& vs){

	GLuint id = vs.getID();

	if(gl.state.vao != id){
		gl.BindVertexArray(id);
		gl.state.vao = id;

		if(IndexBuffer* ib = vs.getIndexBuffer()){
			gl.state.ibo = ib->getID();
		}

		gl.state.enabled_attrib_arrays = vs.getEnabledArrays();
	}
}

VertexState::VertexState()
: enabled_arrays()
, active_attribs()
, vertex_buffers()
, index_buffer(nullptr)
, id(0)
, using_vao(gl.GenVertexArrays != nullptr) {
	if(using_vao) gl.GenVertexArrays(1, &id);
}

VertexState::VertexState(std::initializer_list<VertexBuffer*> bufs, IndexBuffer* idx)
: enabled_arrays()
, active_attribs()
, vertex_buffers()
, index_buffer(nullptr)
, id(0)
, using_vao(gl.GenVertexArrays != nullptr) {
	if(using_vao) gl.GenVertexArrays(1, &id);
	setVertexBuffers(bufs);
	setIndexBuffer(idx);
}

void VertexState::setVertexBuffers(std::initializer_list<VertexBuffer*> buffers){
	if(using_vao) bind_vao(*this);

	GLint vbo_bind_point = 0;
	for(auto* buf : buffers){
		if(using_vao && gl.BindVertexBuffer){
			DEBUGF(
				"BindVertexBuffer: bind_point: %d, id: %d, stride: %d.", 
				vbo_bind_point, buf->getID(), buf->getStride()
			);
			gl.BindVertexBuffer(vbo_bind_point++, buf->getID(), 0, buf->getStride());
		}
		vertex_buffers.push_back(buf);
	}
}

void VertexState::setIndexBuffer(IndexBuffer* buff){
	// bind the index buffer upfront if using VAOs, otherwise do it in bind().

	if(using_vao && buff){
		bind_vao(*this);
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff->getID());
		gl.state.ibo = buff->getID();
	}
	
	index_buffer = buff;
}

void VertexState::setAttribArrays(const ShaderAttribs& new_attribs){
	//TODO: give ShaderAttribs some quick == function.
	if(using_vao) bind_vao(*this);
	
	uint16_t new_enabled_arrays = 0;
	
	TRACEF("Need to set %td attribs.", std::distance(new_attribs.begin(), new_attribs.end()));

	auto& search_attribs = using_vao ? this->active_attribs : gl.state.active_attribs;
	auto& attrib_arrays =  using_vao ? this->enabled_arrays : gl.state.enabled_attrib_arrays;

	for(const auto& a : new_attribs){
		if(using_vao && search_attribs.containsAttrib(a.name_hash, a.index)){
			new_enabled_arrays |= (1 << a.index);
			TRACEF("Attrib %#x already active, skip.", a.name_hash);
			continue;
		}
		
		GLint vbo_bind_point = 0;
		for(auto* vb : vertex_buffers){
			DEBUGF("Checking buffer %p for attrib %u...", vb, a.name_hash);
			const ShaderAttribs& vb_attribs = vb->getShaderAttribs();
			
			DEBUGF("Num attribs in buffer: %td.", 
				std::distance(vb_attribs.begin(), vb_attribs.end()));
			
			if(vb_attribs.containsAttrib(a.name_hash, -1)){
				if(using_vao && gl.VertexAttribBinding){
					DEBUGF("Setting attrib-buffer binding: [%d] -> [%d]", 
						a.index, vbo_bind_point);
				
					gl.VertexAttribBinding(a.index, vbo_bind_point);
				} else {
					DEBUGF("Binding buffer %p for attrib %u", vb, a.name_hash);
					vb->bind();
				}
				
				vb_attribs.bind(a.name_hash, a.index, vb->getStride());
				
				if(using_vao || !(gl.state.enabled_attrib_arrays & (1 << a.index))){
					gl.EnableVertexAttribArray(a.index);
				}
				new_enabled_arrays |= (1 << a.index);
				
				break;
			}
			vbo_bind_point++;
		}
	}

	uint16_t enabled_array_diff = attrib_arrays & ~new_enabled_arrays;
	
	for(size_t i = 0; i < 16; ++i){
		if((enabled_array_diff >> i) & 1){
			gl.DisableVertexAttribArray((GLint)i);
		}
	}
	
	search_attribs = new_attribs;
	attrib_arrays = new_enabled_arrays;
	gl.state.enabled_attrib_arrays = new_enabled_arrays;
}

IndexBuffer* VertexState::getIndexBuffer(void) {
	return index_buffer;
}

void VertexState::bind(){

	if(using_vao) bind_vao(*this);
	
	for(auto* vb : vertex_buffers){
		vb->update();
	}
	
	if(index_buffer){
		if(!using_vao) index_buffer->bind();
		index_buffer->update();
	}
}

void VertexState::onGLContextRecreate(){
	GLuint new_id = 0;

	enabled_arrays = 0;
	active_attribs.clear();

	if(using_vao){
		gl.GenVertexArrays(1, &new_id);
		gl.BindVertexArray(new_id);
	}

	DEBUGF("Reloading VState: vao id [%d] -> [%d].", id, new_id);
	id = new_id;

	GLint vbo_bind_point = 0;
	for(auto* buf : vertex_buffers){
		gl.validateObject(*buf);
		if(gl.BindVertexBuffer){
			DEBUGF(
				"BindVertexBuffer: bind_point: %d, id: %d, stride: %d.", 
				vbo_bind_point, buf->getID(), buf->getStride()
			);
			gl.BindVertexBuffer(vbo_bind_point++, buf->getID(), 0, buf->getStride());
		}
	}

	if(index_buffer) gl.validateObject(*index_buffer);
}

VertexState::~VertexState(){
	if(using_vao && id && gl.initialized()){
		gl.DeleteVertexArrays(1, &id);
	}
}

GLuint VertexState::getID() const {
	return id;
}

uint16_t VertexState::getEnabledArrays() const {
	return enabled_arrays;
}
