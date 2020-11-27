#version 330 core
in vec2 UV;
in vec3 Norm;

uniform int type;
uniform sampler2D HDRI;

vec3 SimpleSky(vec3 dir) {
	vec3 unit_direction = normalize(dir);
	float t = 0.5 * (unit_direction.y + 1.0);
	return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

vec3 SampleSphericalMap(vec3 dir) {
	vec2 invAtan = vec2(0.1591, 0.3183);

	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
	uv *= invAtan; uv.x += 0.5; uv.y += 0.5;

	uv.y = 1.0 - uv.y;
	return texture2D(HDRI, uv).rgb;
}

void main() {
	if (type == 3) 
		gl_FragColor = vec4(pow(SimpleSky(normalize(Norm)), vec3(2.2)), 1.0);
	else 
		gl_FragColor = vec4(SampleSphericalMap(normalize(Norm)), 1.0);
}
