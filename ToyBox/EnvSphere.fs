#version 330 core
in vec2 UV;

uniform sampler2D HDRI;

void main() {
	vec3 base = texture2D(HDRI, UV).rgb;
	gl_FragColor = vec4(base, 1.0);
}