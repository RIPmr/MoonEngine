#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;

uniform float _fishEye;
uniform float _chromatic;
uniform float _scale;

void main() {
	vec2 normUV = (TexCoords * 2.0 - 1.0) * _scale;

	// screen deformation
    vec2 uv = (normUV + 1.0) / 2.0 + normUV * dot(normUV, normUV) * 0.1 * _fishEye;
    vec3 color = texture(screenBuffer, uv).rgb;

	// chromatic aberration
    vec2 mo = normUV * 0.01 * _chromatic;
    FragColor.r = texture(screenBuffer, uv - mo * 0.1, 0.0).r;
    FragColor.g = texture(screenBuffer, uv - mo * 0.6, 0.0).g;
    FragColor.b = texture(screenBuffer, uv - mo * 1.0, 0.0).b;

    FragColor.a = 1.0;
}