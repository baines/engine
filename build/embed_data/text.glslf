#version 330 core

uniform sampler2D u_samp;

flat in float width;
flat in vec2 tex;
in vec2 pos;

out vec4 color;

void main(void){

	ivec2 p = ivec2(tex + fract(pos) * width);
	vec4 pixel = texelFetch(u_samp, p, 0);
	
	color = vec4(0.0f, pixel.r, 0.0f, 1.0f);
}
