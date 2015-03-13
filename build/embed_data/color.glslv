#version 120

uniform mat4 u_ortho;

attribute vec2 a_pos;
attribute vec4 a_col;

varying vec4 col;

void main(){
	col = a_col;
	gl_Position = u_ortho * vec4(a_pos, 0.0, 1.0);
}

