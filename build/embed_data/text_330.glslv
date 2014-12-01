#version 330 core

uniform mat4 u_mvp;

in vec2 a_pos;
in vec2 a_tex;
in float a_width;

flat out float width;
flat out vec2 tex;
out vec2 pos;

void main(void){

	gl_Position = u_mvp * vec4(a_pos, 0, 1);
	
	width = a_width;
	tex = a_tex;
	pos = vec2(floor(gl_VertexID/2), mod(gl_VertexID, 2));
}

