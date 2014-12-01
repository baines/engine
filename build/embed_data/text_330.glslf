#version 330 core

uniform sampler2D u_samp;
uniform float u_height;

flat in float width;
flat in vec2 tex;
in vec2 pos;

out vec4 color;

void main(void){

	vec4 alpha = texture(u_samp, tex + fract(pos) * vec2(width, u_height));
	
	color = vec4(alpha.rrr, 1.0f);
}

