#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;
uniform sampler2D depthBuffer;

// global params
uniform bool debug;
uniform int _type;

// box & gaussian params
uniform int _iter;
uniform float _sigma;

// DOF params
uniform float _distance;
uniform float _multiply;
uniform float _tolerance;
uniform float _cutoff;

// bokeh params
uniform float _time;
uniform float _expo;
uniform float GOLDEN_ANGLE;

// Bokeh ------------------------------------------------------------------------------------
vec3 Bokeh(float radius) {
    float r = 1.0, ITERATIONS = _iter * 30;
	vec3 acc = vec3(0), div = acc;
    vec2 vangle = vec2(0.0, radius * 0.01 / sqrt(ITERATIONS));
	mat2 rot = mat2(cos(GOLDEN_ANGLE), sin(GOLDEN_ANGLE), -sin(GOLDEN_ANGLE), cos(GOLDEN_ANGLE));
    
	for (int j = 0; j < ITERATIONS; j++) {  
        // the approx increase in the scale of sqrt(0, 1, 2, 3...)
        r += 1.0 / r;
	    vangle = rot * vangle;
		vec2 offset = (r - 1.0) * vangle;
		offset.y *= bufferSize.x / bufferSize.y;
        vec3 col = texture(screenBuffer, TexCoords + offset).rgb;
		// Contrast it for better highlights - leave this out elsewhere.
        col = col * col * _expo;
		vec3 bokeh = pow(col, vec3(4));
		acc += col * bokeh;
		div += bokeh;
	}

	return acc / div;
}

// Box Blur ---------------------------------------------------------------------------------
vec3 BoxBlur() {
	vec3 final_colour = vec3(0.0);

	int cnt = 0;
	for (int i = -_iter; i <= _iter; ++i) {
		for (int j = -_iter; j <= _iter; ++j) {
			final_colour += texture(screenBuffer, TexCoords + vec2(float(i), float(j)) / bufferSize).rgb;
			cnt++;
		}
	}

	return final_colour / cnt;
}

// Gaussian Blur ----------------------------------------------------------------------------
float normpdf(float x, float sigma) {
	return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float SampleDepth(vec2 uv) {
	float depth = 2 * texture(depthBuffer, uv).r - 1;
	return depth;
}

vec3 GaussianBlur(int iter, float depth) {
	// declare stuff
	if (iter > 16) iter = 16;
	int iSize = iter * 2 + 1;
		
	// create the 1-D kernel
	float kernel[129]; // kernel size = iSize, iSize max = 64
	for (int j = 0; j <= iter; ++j) {
		kernel[iter + j] = kernel[iter - j] = normpdf(float(j), _sigma);
	}
		
	// get the normalization factor (as the gaussian has been clamped)
	float Z = 0.0;
	for (int j = 0; j < iSize; ++j) {
		Z += kernel[j];
	}
		
	// read out the texels
	vec3 final_colour = vec3(0.0);
	for (int i = -iter; i <= iter; ++i) {
		for (int j = -iter; j <= iter; ++j) {
			vec2 suv = TexCoords + vec2(float(i), float(j)) / bufferSize;
			bool adj = abs(SampleDepth(suv) - depth) * 10 <= _tolerance;

			vec3 color = texture(screenBuffer, adj ? suv : TexCoords).rgb;
			//vec3 color = texture(screenBuffer, suv).rgb;
			final_colour += kernel[iter + j] * kernel[iter + i] * color;
		}
	}

	return final_colour / (Z * Z);
}

float BlurredDepth(int iter) {
	// declare stuff
	if (iter > 16) iter = 16;
	int iSize = iter * 2 + 1;
		
	// create the 1-D kernel
	float kernel[129]; // kernel size = iSize, iSize max = 64
	for (int j = 0; j <= iter; ++j) {
		kernel[iter + j] = kernel[iter - j] = normpdf(float(j), _sigma);
	}
		
	// get the normalization factor (as the gaussian has been clamped)
	float Z = 0.0;
	for (int j = 0; j < iSize; ++j) {
		Z += kernel[j];
	}
		
	// read out the texels
	float final_colour = 0.0;
	for (int i = -iter; i <= iter; ++i) {
		for (int j = -iter; j <= iter; ++j) {
			vec2 suv = TexCoords + vec2(float(i), float(j)) / bufferSize;
			float color = texture(depthBuffer, suv).r;
			final_colour += kernel[iter + j] * kernel[iter + i] * color;
		}
	}
	
	return 2 * final_colour / (Z * Z) - 1;
}

void main() {
	float depth = pow(SampleDepth(TexCoords), _iter);
	//float depth = pow(BlurredDepth(int(_time)), _iter) * _multiply;

	if (debug) {
		vec3 color = texture(screenBuffer, TexCoords).rgb;
		if (abs(depth - _distance) <= _cutoff / 10.0) FragColor = vec4(vec3(0, 1, 0) * color, 1.0);
		else FragColor = vec4(color, 1.0);
	} else {
		FragColor = vec4(GaussianBlur(int(depth * _multiply), depth), 1.0);
	}
}