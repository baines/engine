#version 330 core

uniform float u_height = 16.0f;
uniform mat4 u_mvp;

in vec2 a_pos;
in vec4 a_tex;

flat out float width;
flat out vec2 tex;
out vec2 pos;

void main(void){

	gl_Position = u_mvp * vec4(a_pos, 0, 1);
	
	width = a_tex.w;
	tex = vec2(a_tex.x * 256 + a_tex.y, a_tex.z * u_height);
	pos = vec2(gl_VertexID/2, gl_VertexID);
}
