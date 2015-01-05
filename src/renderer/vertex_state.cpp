#include "vertex_state.h"

VertexState::VertexState()
: enabled_arrays()
, active_attrs()
, vertex_buffers()
, index_buffer(nullptr)
, id(0) {
	gl.GenVertexArrays(1, &id);
}

void VertexState::setVertexBuffers(std::initializer_list<VertexBuffer*> buffers){
	gl.BindVertexArray(id);
	
	GLint vbo_bind_point = 0;
	for(auto* buf : buffers){
		DEBUGF(
			"BindVertexBuffer: bind_point: %d, id: %d, stride: %d.", 
			vbo_bind_point, buf->getID(), buf->getStride()
		);
		gl.BindVertexBuffer(vbo_bind_point++, buf->getID(), 0, buf->getStride());
		vertex_buffers.push_back(buf);
	}
}

void VertexState::setIndexBuffer(IndexBuffer* buff){
	gl.BindVertexArray(id);
	buff->bind();
	
	index_buffer = buff;
}

void VertexState::setAttribArrays(RenderState& rs, const ShaderAttribs& new_attrs){
	//TODO: give ShaderAttribs some quick == function.
	bind(rs);
		
	std::bitset<16> new_enabled_arrays;
	
	TRACEF("Need to set %ld attribs.", std::distance(new_attrs.begin(), new_attrs.end()));
	
	for(const auto& a : new_attrs){
		if(active_attrs.containsAttrib(a.name_hash, a.index)){
			new_enabled_arrays[a.index] = 1;
			TRACEF("Attrib %#x already active, skip.", a.name_hash);
			continue;
		}
		
		GLint vbo_bind_point = 0;
		for(auto* vb : vertex_buffers){
			DEBUGF("Checking buffer %p for attrib %ld...", vb, a.name_hash);
			const ShaderAttribs& vb_attrs = vb->getShaderAttribs();
			
			DEBUGF("Num attribs in buffer: %ld.", std::distance(vb_attrs.begin(), vb_attrs.end()));
			
			if(vb_attrs.containsAttrib(a.name_hash, -1)){
				vb_attrs.bind(a.name_hash, a.index);
				
				DEBUGF("Setting attrib-buffer binding: [%d] -> [%d]", a.index, vbo_bind_point);
				
				gl.VertexAttribBinding(a.index, vbo_bind_point);
				gl.EnableVertexAttribArray(a.index);
				
				new_enabled_arrays[a.index] = 1;
				
				break;
			}
			vbo_bind_point++;
		}
	}

	auto enabled_array_diff = enabled_arrays & ~new_enabled_arrays;
	
	for(size_t i = 0; i < enabled_array_diff.size(); ++i){
		if(enabled_array_diff[i]){
			gl.DisableVertexAttribArray((GLint)i);
		}
	}
	
	active_attrs = new_attrs;
	enabled_arrays = new_enabled_arrays;
}

IndexBuffer* VertexState::getIndexBuffer(void){
	return index_buffer;
}

void VertexState::bind(RenderState& rs){
	if(rs.vao != id){
		gl.BindVertexArray(id);
		rs.vao = id;
	}
	
	for(auto* vb : vertex_buffers){
		vb->update();
	}
}

void VertexState::onGLContextRecreate(){
	int old_id = id;

	enabled_arrays.reset();
	active_attrs.clear();

	gl.GenVertexArrays(1, &id);
	gl.BindVertexArray(id);

	DEBUGF("Reloading VState: vao id [%d] -> [%d].", old_id, id);

	GLint vbo_bind_point = 0;
	for(auto* buf : vertex_buffers){
		gl.validateObject(*buf);
		DEBUGF(
			"BindVertexBuffer: bind_point: %d, id: %d, stride: %d.", 
			vbo_bind_point, buf->getID(), buf->getStride()
		);
		gl.BindVertexBuffer(vbo_bind_point++, buf->getID(), 0, buf->getStride());
	}

	if(index_buffer) gl.validateObject(*index_buffer);
}

VertexState::~VertexState(){
	if(id && gl.initialized()){
		gl.DeleteVertexArrays(1, &id);
	}
}
