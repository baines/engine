#version 120

uniform float timer;
uniform float height;
varying vec2 pos;

float compat_round(float v){
	return floor(v + 0.5);
}

void main(){

	vec2 intensity = vec2(
		sin(mod(timer, 62.8) / 10.0) / 5.0,
		0.75
	);

	vec2 line = vec2(
		compat_round(mod(pos.x, 10.0) / 10.0),
		compat_round(mod(pos.y - timer, 10.0) / 10.0)
	);

	vec4 colour = vec4(
		0.2 + 0.05 - intensity.x,
		0.2 + intensity.x * line.x,
		0.3 + intensity.y * line.y,
		0.4 + (intensity.x / 5.0) * line.x + (intensity.y / 5.0) * line.y
	);

	float border = clamp((pos.y + 4) - height, 0.0, 1.0);

	gl_FragColor = max(border * vec4(0.2, 0.2, 0.7, 1.0), (1.0 - border) * colour);

}
