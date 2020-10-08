#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform float gamma; // default 2.2
uniform float exposure;
uniform sampler2D screenBuffer;

void main() {
    vec3 hdrColor = texture(screenBuffer, TexCoords).rgb;
	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

	// Gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}