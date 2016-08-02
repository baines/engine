#version 120

uniform sampler2D u_samp;

varying vec2 tex;
varying vec4 col;

void main(){
	gl_FragColor = col * texture2D(u_samp, tex);
}

