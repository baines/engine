#version 330 core

uniform float timer = 1.0f;

in vec2 a_pos;
in vec4 a_col;

out vec3 pass_col;

void main(){
	gl_Position = vec4(vec2(a_pos.xy * timer), 0, 1);
	pass_col = a_col.xyz;
}

