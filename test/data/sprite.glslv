#version 120

uniform mat4 u_ortho;

attribute vec2 a_pos;
attribute vec2 a_tex;

varying vec2 tex;

void main(){
	tex = a_tex;
	gl_Position = u_ortho * vec4(a_pos, 0.0, 1.0);
}
