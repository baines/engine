#version 120

uniform sampler2D u_samp;

varying vec2 tex;
varying vec4 col;

void main(void){

	vec4 alpha = texture2D(u_samp, tex);
	
	gl_FragColor = vec4(col.r, col.g, col.b, alpha.a * col.a);
}

