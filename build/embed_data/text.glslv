#version 120

uniform mat4 u_mvp;

attribute vec2 a_pos;
attribute vec2 a_tex;

varying vec2 tex;

void main(void){
	tex = a_tex;
	gl_Position = u_mvp * vec4(a_pos, 0, 1);
}

