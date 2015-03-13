#version 120

uniform float timer;

attribute vec2 a_pos;
attribute vec4 a_col;

varying vec3 pass_col;

void main(){
	gl_Position = vec4(vec2(a_pos.xy * timer), 0.0, 1.0);
	pass_col = a_col.xyz;
}

