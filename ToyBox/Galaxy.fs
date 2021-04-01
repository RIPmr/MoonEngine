#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform mat4 invPVMat;
uniform sampler2D screenBuffer;
uniform sampler2D depthBuffer;

uniform vec3 bboxMin;
uniform vec3 bboxMax;

// noise params
uniform vec3 _offset;
uniform float _scale;
uniform vec2 _noiseMulti;
uniform float _time;

// parameters
uniform float _step;
uniform int _maxMarchLoop;
uniform float _rayStep;
uniform float _lightStep;

uniform float _darknessThreshold;
uniform float _midtoneOffset;
uniform float _shadowOffset;
uniform float _lightAbsorptionTowardSun;
uniform float _lightAbsorptionThroughCloud;
uniform vec3 _mainColor;
uniform vec3 _shadowColor;
uniform vec4 _phaseParams;

uniform float _scatterMultiply;
uniform float _densityOffset;
uniform float _densityMultiply;

uniform int _downSampling;

// global params
vec3 viewPos, viewDir;

vec2 RayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
	vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
	vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
	vec3 tmin = min(t0, t1);
	vec3 tmax = max(t0, t1);

	// enter point
	float dstA = max(max(tmin.x, tmin.y), tmin.z);
	// exit point
	float dstB = min(tmax.x, min(tmax.y, tmax.z));

	float dstToBox = max(0, dstA);
	float dstInsideBox = max(0, dstB - dstToBox);
	return vec2(dstToBox, dstInsideBox);
}

void CalculatePixelRay(vec2 n) {
	vec4 ray_start = invPVMat * vec4(n.x, n.y, -1.0, 1.0);
	vec4 ray_end = invPVMat * vec4(n.x, n.y, 1.0, 1.0);

	ray_start.xyz /= ray_start.w;
	ray_end.xyz /= ray_end.w;

	viewPos = ray_start.xyz;
	viewDir = normalize((ray_end - ray_start).xyz);
}

vec4 CalculateWorldSpacePosition(float depth, vec2 n) {
	return invPVMat * vec4(n.x, n.y, depth, 1.0);
}

vec2 csqr(vec2 a) { return vec2(a.x * a.x - a.y * a.y, 2.0 * a.x * a.y); }

float map(vec3 p) {
	p = (p + _offset - (bboxMax + bboxMin) / 2.0) * _scale;
	float res = 0.0; vec3 c = p;
	for (int i = 0; i < _lightStep; ++i) {
        p = _phaseParams.x * abs(p) / dot(p, p) - _phaseParams.y;
        p.yz = csqr(p.yz); p = p.zxy;
        res += exp(-_step * 32.0 * abs(dot(p, c)));
	}
	return res * _densityMultiply * _step * 32.0 / 38.0;
}

float GetDensity(float _origin) {
	float invCol = (1.0 - _origin) * 3.0;
	return mix(
		clamp(invCol, 1.0, 10.0),
		min(1.0, invCol),
		_scatterMultiply
	);
}

vec3 ColorByDensity(float dens, vec3 hueShift) {
	return vec3(
		pow(dens, GetDensity(hueShift.r)), 
		pow(dens, GetDensity(hueShift.g)), 
		pow(dens, GetDensity(hueShift.b))
	);
}

vec3 raymarch(vec3 ro, vec3 rd, vec3 worldPos) {
	// calculate intersect info
	float depthEyeLinear = length(worldPos - ro);
	vec2 intersectInfo = RayBoxDst(bboxMin, bboxMax, ro, 1.0 / viewDir);
	float dstLimit = min(depthEyeLinear - intersectInfo.x, intersectInfo.y);
	
    //float dt = 0.05; // static
    //float dt = 0.2 - 0.195 * cos(time * 0.05); // loop anime
    float dt = _phaseParams.z - 0.195 * _time;
    float t = intersectInfo.x, c = 0.0;
    vec3 col = vec3(0.0);
	if (dstLimit > 0) {
		for (int i = 0; i < _maxMarchLoop; i++) {
			t += dt * _rayStep * exp(-2.0 * c);
			if (t - intersectInfo.x > dstLimit) break;
			else {
				c = pow(map(ro + t * rd), _densityOffset);

				// tone mapping
				vec3 mappedColor = mix(
					_shadowColor, _mainColor, 
					clamp(pow(c * _shadowOffset, _darknessThreshold * 128.0), 0.0, 1.0)
				);

				col = 0.99 * col + 0.08 * ColorByDensity(c, mappedColor);
			}
		}
	}
    return col;
}

void main() {
	// depth to world pos
	float depth = 2 * texture(depthBuffer, TexCoords).r - 1;
	vec4 worldPos = CalculateWorldSpacePosition(depth, TexCoords * 2.0 - vec2(1.0));

	// raymarch
	CalculatePixelRay(TexCoords * 2.0 - vec2(1.0));
    vec3 col = raymarch(viewPos, viewDir, worldPos.xyz / worldPos.w);
	
	// shade
    vec3 bg = texture(screenBuffer, TexCoords).rgb;
	float dens = clamp((col.r + col.g + col.b) / 3.0, 0.0, 1.0);
    FragColor = vec4(mix(bg, col, dens), 1.0);
}