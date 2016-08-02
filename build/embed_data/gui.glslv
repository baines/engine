#version 120

uniform mat4 u_ortho;

attribute vec2 a_pos;
attribute vec2 a_tex;
attribute vec4 a_col;

varying vec2 tex;
varying vec4 col;

void main(){
	tex = a_tex;
	col = a_col;
	gl_Position = u_ortho * vec4(a_pos.xy, 0, 1);
}

