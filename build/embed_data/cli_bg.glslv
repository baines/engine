#version 120

uniform mat4 u_ortho;

attribute vec2 a_pos;

varying vec2 pos;

void main(){
	pos = a_pos;
	gl_Position = u_ortho * vec4(a_pos, 0, 1);
}
