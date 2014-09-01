#ifndef VERTEX_STATE_H_
#define VERTEX_STATE_H_
#include "gl_context.h"
#include "vertex_buffer.h"
#include "index_buffer.h"

struct VertexState {

	VertexState(){
		gl.GenVertexArrays(1, &id);
	}

	void setAttribData(int index, VertexBuffer* buff, int buff_index){
		gl.BindVertexArray(id);
		
		gl.EnableVertexAttribArray(index);
		buff->applyFormat(index, buff_index);
		gl.VertexAttribBinding(index, buff->getID());
	}
	
	void setAttribIndices(int index, IndexBuffer* buff){
		gl.BindVertexArray(id);
		
		/*attrib[index].index_buff = buff;
		buff->bind();*/
	}
	
	IndexBuffer* getIndexBuffer(void){
		return ib;
	}
	
	void bind(RenderState& rs){
		if(rs.vao != id){
			gl.BindVertexArray(id);
			rs.vao = id;
		}
	}
private:
	VertexBuffer* vb;
	IndexBuffer* ib;
	GLuint id;
};

#endif
