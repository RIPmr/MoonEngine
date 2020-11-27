#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;

uniform float _time;

// The Golden Angle is (3.0 - sqrt(5.0)) * PI radians, which doesn't precompiled for some reason.
#define GOLDEN_ANGLE 2.39996
#define ITERATIONS 150

mat2 rot = mat2(cos(GOLDEN_ANGLE), sin(GOLDEN_ANGLE), -sin(GOLDEN_ANGLE), cos(GOLDEN_ANGLE));

vec3 Bokeh(sampler2D tex, vec2 uv, float radius) {
	vec3 acc = vec3(0), div = acc;
    float r = 1.0;
    vec2 vangle = vec2(0.0, radius * 0.01 / sqrt(float(ITERATIONS)));
    
	for (int j = 0; j < ITERATIONS; j++) {  
        // the approx increase in the scale of sqrt(0, 1, 2, 3...)
        r += 1.0 / r;
	    vangle = rot * vangle;
        vec3 col = texture(tex, uv + (r - 1.0) * vangle).xyz; // Sample the image
        col = col * col * 1.8; // Contrast it for better highlights - leave this out elsewhere.
		vec3 bokeh = pow(col, vec3(4));
		acc += col * bokeh;
		div += bokeh;
	}

	return acc / div;
}

void main() {
	float rad = 0.8 - 0.8 * cos(_time * 6.283);

    FragColor = vec4(Bokeh(screenBuffer, TexCoords, rad), 1.0);
}