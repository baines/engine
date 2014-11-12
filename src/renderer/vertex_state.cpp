#include "vertex_state.h"

VertexState::VertexState()
: enabled_arrays() {
	gl.GenVertexArrays(1, &id);
}

void VertexState::setVertexBuffers(std::initializer_list<VertexBuffer*> buffers){
	gl.BindVertexArray(id);
	
	GLint vbo_bind_point = 0;
	for(auto* buf : buffers){
		gl.BindVertexBuffer(vbo_bind_point++, buf->getID(), 0, buf->getStride());
		vertex_buffers.push_back(buf);
	}
}

void VertexState::setIndexBuffer(IndexBuffer* buff){
	gl.BindVertexArray(id);
	buff->bind();
	
	index_buffer = buff;
}

void VertexState::setAttribArrays(const ShaderAttribs& new_attrs){
	gl.BindVertexArray(id);
	
	std::bitset<16> new_enabled_arrays;
	
	for(const auto& a : new_attrs){
		if(active_attrs.containsAttrib(a.name_hash, a.index)) continue;
		
		GLint vbo_bind_point = 0;
		for(auto* vb : vertex_buffers){
			const ShaderAttribs& vb_attrs = vb->getShaderAttribs();
			
			if(vb_attrs.containsAttrib(a.name_hash, a.index)){
				vb_attrs.bind(a.name_hash);
				
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