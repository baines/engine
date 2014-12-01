#version 120

uniform sampler2D u_samp;

varying vec2 tex;
varying vec2 pos;

void main(void){

	vec4 alpha = texture2D(u_samp, tex);
	
	gl_FragColor = vec4(alpha.aaa, 1.0f);
}

