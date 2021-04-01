#version 330 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D environmentMap;

const float PI = 3.14159265359;
const vec2 invAtan = vec2(0.1591, 0.3183);

vec3 SampleSphericalMap(vec3 dir) {
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
	uv *= invAtan; uv.x += 0.5; uv.y += 0.5;

	//uv.y = 1.0 - uv.y;
	return texture2D(environmentMap, uv).rgb;
}

vec3 UVtoDirection(vec2 uv) {
	bool flag = (uv.x < 0.25 || uv.x > 0.75) ? true : false;
	uv.x -= 0.5; uv.y -= 0.5;
	uv /= invAtan;

	vec3 dir;
	dir.y = sin(uv.y);
	float t = tan(uv.x);
	dir.x = sqrt((1 - dir.y * dir.y) / (1 + t * t));
	if (flag) dir.x *= -1;
	dir.z = dir.x * t;
	
	return dir;
}

void main() {
	// The world vector acts as the normal of a tangent surface from the origin, aligned to WorldPos. 
	// Given this normal, calculate all incoming radiance of the environment. 
	// The result of this radiance is the radiance of light coming from -Normal direction, 
	// which is what we use in the PBR shader to sample irradiance.
	vec3 WorldPos = UVtoDirection(UV);
    vec3 N = normalize(WorldPos);

    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float nrSamples = 0.0;
    float sampleDelta = 0.025;
    vec3 irradiance = vec3(0.0);
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			tangentSample = normalize(tangentSample);

            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += SampleSphericalMap(normalize(sampleVec)) * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}