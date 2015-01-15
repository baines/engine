#version 120

uniform sampler2D u_samp;

varying vec2 tex;

void main(){
	gl_FragColor = texture2D(u_samp, tex);
}
