#version 120

uniform vec4 u_ortho;

attribute vec2 a_pos;
attribute vec4 a_col;

varying vec4 col;

void main(){
	col = a_col;
	gl_Position = u_ortho * a_pos;
}

