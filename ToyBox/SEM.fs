#version 330 core
in vec3 vN;
in vec3 eye;

uniform sampler2D tMatCap;

void main() {
	// r = e - 2.0 * dot(n, e) * n;
	vec3 r = reflect(normalize(eye), normalize(vN));
	// m = 2.0 * sqrt(pow(r.x, 2.0) + pow(r.y, 2.0) + pow(r.z + 1.0, 2.0));
	float m = 2.82842712474619 * sqrt(r.z + 1.0);

	vec3 base = texture2D(tMatCap, r.xy / m + 0.5).rgb;
	gl_FragColor = vec4(base, 1.0);
}