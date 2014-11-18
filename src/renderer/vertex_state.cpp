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
		DEBUGF("BindVertexBuffer: bind_point: %d, id: %d, stride: %d\n", 
		vbo_bind_point, buf->getID(), buf->getStride());
		gl.BindBuffer(GL_ARRAY_BUFFER, buf->getID());
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
	
	DEBUGF("attr size: %ld\n", std::distance(new_attrs.begin(), new_attrs.end()));
	
	for(const auto& a : new_attrs){
		if(active_attrs.containsAttrib(a.name_hash, a.index)){
			new_enabled_arrays[a.index] = 1;
			DEBUGF("contains %d, skip\n", a.name_hash);
			continue;
		}
		
		GLint vbo_bind_point = 0;
		for(auto* vb : vertex_buffers){
			DEBUGF("checking buffer\n");
			const ShaderAttribs& vb_attrs = vb->getShaderAttribs();
			
			DEBUGF("attrs in buffer: %ld\n", std::distance(vb_attrs.begin(), vb_attrs.end()));
			
			if(vb_attrs.containsAttrib(a.name_hash, -1)){
				vb_attrs.bind(a.name_hash, a.index);
				
				DEBUGF("Set attr bind: a_i [%d] -> vbo_i [%d]\n", a.index, vbo_bind_point);
				
				gl.VertexAttribBinding(a.index, vbo_bind_point);
				gl.EnableVertexAttribArray(a.index);
				
				new_enabled_arrays[a.index] = 1;
				
				break;
			}
			vbo_bind_point++;
		}
	}

	auto enabled_array_diff = enabled_arrays & ~new_enabled_arrays;
	
	for(GLint i = 0; i < enabled_array_diff.size(); ++i){
		if(enabled_array_diff[i]){
			gl.DisableVertexAttribArray(i);
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
}
