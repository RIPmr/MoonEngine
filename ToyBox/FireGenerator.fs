#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
//uniform sampler2D screenBuffer;
uniform float scale;
uniform vec2 offset;
uniform float power;
uniform float time;

uniform vec3 tint;

vec2 hash(vec2 p) {
	p = vec2(
		dot(p, vec2(127.1, 311.7)),
		dot(p, vec2(269.5, 183.3))
	);
	return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float noise(vec2 p) {
	const float K1 = 0.366025404; // (sqrt(3) - 1) / 2;
	const float K2 = 0.211324865; // (3 - sqrt(3)) / 6;
	
	vec2 i = floor(p + (p.x + p.y) * K1);
	
	vec2 a = p - i + (i.x + i.y) * K2;
	vec2 o = (a.x > a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec2 b = a - o + K2;
	vec2 c = a - 1.0 + 2.0 * K2;
	
	vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
	
	vec3 n = pow(h, vec3(4.0)) * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
	
	return dot(n, vec3(70.0));
}

float fbm(vec2 uv) {
	float f;
	mat2 m = mat2(1.6, 1.2, -1.2, 1.6);
	f  = 0.5000 * noise(uv); uv = m * uv;
	f += 0.2500 * noise(uv); uv = m * uv;
	f += 0.1250 * noise(uv); uv = m * uv;
	f += 0.0625 * noise(uv); uv = m * uv;
	f = 0.5 + 0.5 * f;
	return f;
}

void main() {
	vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
	vec2 q = (uv + offset) * scale; q.x *= 5.0; q.y *= 2.0;
	float strength = floor(q.x + 1.0);
	float T3 = max(3.0, 1.25 * strength) * time;
	q.x = mod(q.x, 1.0) - 0.5; q.y -= 0.25;
	float n = fbm(strength * q - vec2(0, T3));
	float c = 1.0 - 16.0 * pow( 
		max(0.0, length(q * vec2(1.8 + q.y * 1.5, 0.75)) - 
		n * max(0.0, q.y + 0.25)), 1.2
	);
	float c1 = n * c * (1.5 - pow(2.50 * uv.y, 4.0));
	c1 = clamp(c1, 0.0, 1.0);

	//vec3 col = vec3(1.5 * c1, 1.5 * pow(c1, 3), pow(c1, 6));
	vec3 col = vec3(
		clamp(tint.r * 3.0, 1.0, 1.5) * pow(c1, (1.0 - tint.r) * 6),
		clamp(tint.g * 3.0, 1.0, 1.5) * pow(c1, (1.0 - tint.g) * 6),
		clamp(tint.b * 3.0, 1.0, 1.5) * pow(c1, (1.0 - tint.b) * 6)
	) * power;
	
	float a = c * (1.0 - pow(uv.y, 3.0));
	FragColor = vec4(mix(vec3(0.0), col, a), 1.0);
}