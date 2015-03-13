#include "vertex_state.h"

VertexState::VertexState()
: enabled_arrays()
, active_attribs()
, vertex_buffers()
, index_buffer(nullptr)
, id(0)
, using_vao(gl.GenVertexArrays != nullptr) {
	
	if(using_vao) gl.GenVertexArrays(1, &id);
}

void VertexState::setVertexBuffers(std::initializer_list<VertexBuffer*> buffers){
	if(using_vao) gl.BindVertexArray(id);
	
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
		gl.BindVertexArray(id);
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff->getID());
	}
	
	index_buffer = buff;
}

void VertexState::setAttribArrays(RenderState& rs, const ShaderAttribs& new_attribs){
	//TODO: give ShaderAttribs some quick == function.
	if(using_vao && id != rs.vao){
		gl.BindVertexArray(id);
		rs.vao = id;
	}
	
	std::bitset<16> new_enabled_arrays;
	
	TRACEF("Need to set %ld attribs.", std::distance(new_attribs.begin(), new_attribs.end()));

	auto& search_attribs = using_vao ? this->active_attribs : rs.active_attribs;
	auto& attrib_arrays =  using_vao ? this->enabled_arrays : rs.enabled_attrib_arrays;

	for(const auto& a : new_attribs){
		if(using_vao && search_attribs.containsAttrib(a.name_hash, a.index)){
			new_enabled_arrays[a.index] = 1;
			TRACEF("Attrib %#x already active, skip.", a.name_hash);
			continue;
		}
		
		GLint vbo_bind_point = 0;
		for(auto* vb : vertex_buffers){
			DEBUGF("Checking buffer %p for attrib %ld...", vb, a.name_hash);
			const ShaderAttribs& vb_attribs = vb->getShaderAttribs();
			
			DEBUGF("Num attribs in buffer: %ld.", 
				std::distance(vb_attribs.begin(), vb_attribs.end()));
			
			if(vb_attribs.containsAttrib(a.name_hash, -1)){
				if(using_vao && gl.VertexAttribBinding){
					DEBUGF("Setting attrib-buffer binding: [%d] -> [%d]", 
						a.index, vbo_bind_point);
				
					gl.VertexAttribBinding(a.index, vbo_bind_point);
				} else {
					DEBUGF("Binding buffer %p for attrib %u", vb, a.name_hash);
					vb->bind(rs);
				}
				
				vb_attribs.bind(a.name_hash, a.index, vb->getStride());
				
				if(using_vao || !rs.enabled_attrib_arrays[a.index]){
					gl.EnableVertexAttribArray(a.index);
				}
				new_enabled_arrays[a.index] = 1;
				
				break;
			}
			vbo_bind_point++;
		}
	}

	auto enabled_array_diff = attrib_arrays & ~new_enabled_arrays;
	
	for(size_t i = 0; i < enabled_array_diff.size(); ++i){
		if(enabled_array_diff[i]){
			gl.DisableVertexAttribArray((GLint)i);
		}
	}
	
	search_attribs = new_attribs;
	attrib_arrays = new_enabled_arrays;
}

IndexBuffer* VertexState::getIndexBuffer(void){
	return index_buffer;
}

void VertexState::bind(RenderState& rs){
	if(using_vao && rs.vao != id){
		gl.BindVertexArray(id);
		rs.vao = id;
	}
	
	for(auto* vb : vertex_buffers){
		vb->update(rs);
	}
	
	if(index_buffer){
		if(!using_vao) index_buffer->bind(rs);	
		index_buffer->update(rs);
	}
}

void VertexState::onGLContextRecreate(){
	int old_id = id;

	enabled_arrays.reset();
	active_attribs.clear();

	if(using_vao){
		gl.GenVertexArrays(1, &id);
		gl.BindVertexArray(id);
	}

	DEBUGF("Reloading VState: vao id [%d] -> [%d].", old_id, id);

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
