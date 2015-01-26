#version 120

uniform sampler2D u_samp;
uniform vec4 u_outline_col;

varying vec2 tex;
varying vec4 col;

void main(void){

	vec4 alpha = texture2D(u_samp, tex);
	
	gl_FragColor = col * alpha.b + u_outline_col * (alpha.a - alpha.b);
}

