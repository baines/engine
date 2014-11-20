#version 330 core

in vec3 pass_col;

out vec3 color;

void main(){
	color = pass_col;
}

