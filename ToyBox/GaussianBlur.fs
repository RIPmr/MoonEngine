#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;

uniform int _iter;
uniform float _sigma;

float normpdf(float x, float sigma) {
	return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

void main() {
	// declare stuff
	float kernel[129]; // kernel size = iSize, iSize max = 64
	int iSize = _iter * 2 + 1;
	int kSize = (iSize - 1) / 2;
	vec3 final_colour = vec3(0.0);
		
	// create the 1-D kernel
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j) {
		kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), _sigma);
	}
		
	// get the normalization factor (as the gaussian has been clamped)
	for (int j = 0; j < iSize; ++j) {
		Z += kernel[j];
	}
		
	// read out the texels
	for (int i = -kSize; i <= kSize; ++i) {
		for (int j = -kSize; j <= kSize; ++j) {
			final_colour += kernel[kSize + j] * kernel[kSize + i] * 
			texture(screenBuffer, TexCoords + vec2(float(i), float(j)) / bufferSize).rgb;
		}
	}
		
	FragColor = vec4(final_colour / (Z * Z), 1.0);
}