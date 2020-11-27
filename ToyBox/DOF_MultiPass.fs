#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;
uniform sampler2D depthBuffer;

// blur direction
uniform vec2 dir;

// global params
uniform bool debug;

// DOF params
uniform float _iter;
uniform float _falloff;
uniform float _distance;
uniform float _multiply;
uniform float _tolerance;
uniform float _cutoff;

float weight(float x) {
	return 1.0 - x * x * x * x;
}

float SampleDepth(vec2 uv) {
	float depth = 2 * texture(depthBuffer, uv).r - 1;
	depth = abs(_distance - depth);
	if (depth <= _tolerance) depth = pow(depth / _tolerance, _falloff) * _tolerance;
	depth = pow(depth * _multiply, 1.0 / _iter);
	return depth;
}

void main() {
    float dist = SampleDepth(TexCoords);
    float totalw = 0.0;
    
    vec3 color = vec3(0.0);
    for (int i = 0; i <= 20; i++) {
        vec2 p = TexCoords;
        float fi = float(i - 10) / 10.0;
        p.xy += dir * fi * dist;
        
        float w = weight(fi);
        
    	vec4 c = vec4(texture(screenBuffer, p).rgb, SampleDepth(p));
        if (dist >= c.a) w *= max(1.0 - (dist - c.a) / _cutoff, 0.0);
        color += c.rgb * w;
        totalw += w;
    }

    color /= totalw;
	FragColor = vec4(color, 1.0);
}