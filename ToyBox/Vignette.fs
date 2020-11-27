#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;

uniform float _weight;
uniform float _shape;
uniform float _smooth;

void main() {
    vec3 color = texture(screenBuffer, TexCoords).rgb;
	
	// vignette color
    vec2 vi = pow(abs(TexCoords * 2.0 - 1.0), vec2(_shape));
	float vigCol = 1.0 - pow(dot(vi, vi) * 0.45 * _weight, _smooth);

    FragColor = vec4(color.rgb * vigCol, 1.0);
}