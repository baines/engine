#version 120

varying vec3 pass_col;

void main(){
	gl_FragColor = vec4(pass_col, 1.0f);
}

