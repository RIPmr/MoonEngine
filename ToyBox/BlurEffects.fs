#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;

// global params
uniform int _type;

// box & gaussian params
uniform int _iter;
uniform float _sigma;

// bokeh params
uniform float _time;
uniform float _expo;
uniform float GOLDEN_ANGLE;

// Gaussian Blur ----------------------------------------------------------------------------
float normpdf(float x, float sigma) {
	return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

vec3 GaussianBlur(){
	// declare stuff
	int iSize = _iter * 2 + 1;
		
	// create the 1-D kernel
	float kernel[129]; // kernel size = iSize, iSize max = 64
	for (int j = 0; j <= _iter; ++j) {
		kernel[_iter + j] = kernel[_iter - j] = normpdf(float(j), _sigma);
	}
		
	// get the normalization factor (as the gaussian has been clamped)
	float Z = 0.0;
	for (int j = 0; j < iSize; ++j) {
		Z += kernel[j];
	}
		
	// read out the texels
	vec3 final_colour = vec3(0.0);
	for (int i = -_iter; i <= _iter; ++i) {
		for (int j = -_iter; j <= _iter; ++j) {
			final_colour += kernel[_iter + j] * kernel[_iter + i] * 
			texture(screenBuffer, TexCoords + vec2(float(i), float(j)) / bufferSize).rgb;
		}
	}

	return final_colour / (Z * Z);
}

// Box Blur ---------------------------------------------------------------------------------
vec3 BoxBlur(){
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

void main() {
	if (_type == 0) {
		FragColor = vec4(BoxBlur(), 1.0);
	} else if (_type == 1) {
		FragColor = vec4(GaussianBlur(), 1.0);
	} else if (_type == 2) {
		FragColor = vec4(Bokeh(_time), 1.0);
	}
}