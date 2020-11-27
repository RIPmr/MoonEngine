#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform mat4 invPVMat;
uniform sampler2D screenBuffer;
uniform sampler2D depthBuffer;

//uniform vec2 bufferSize;
uniform vec3 bboxMin;
uniform vec3 bboxMax;

// lights
uniform int lightNum;
uniform vec3 lightPositions[32];
uniform vec3 lightColors[32];

// noise params
uniform vec3 _offset;
uniform float _scale;
uniform vec2 _noiseMulti;

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

// macros
#define M_PI 3.14159265358979323846
#define MOD3 vec3(0.1031, 0.11369, 0.13787)

// global params
vec3 viewPos, viewDir;

// noise generator -------------------------------------------------------------
vec3 hash33(vec3 p3) {
	p3 = fract(p3 * MOD3);
    p3 += dot(p3, p3.yxz + 19.19);
    return 2.0 * fract(vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x)) - 1.0;
}

float PerlinNoise3(vec3 p) {
    vec3 pi = floor(p);
    vec3 pf = p - pi;
    
    vec3 w = pf * pf * (3.0 - 2.0 * pf);
    
    return 	mix(
        		mix(
                	mix(dot(pf - vec3(0, 0, 0), hash33(pi + vec3(0, 0, 0))), 
                        dot(pf - vec3(1, 0, 0), hash33(pi + vec3(1, 0, 0))),
                       	w.x),
                	mix(dot(pf - vec3(0, 0, 1), hash33(pi + vec3(0, 0, 1))), 
                        dot(pf - vec3(1, 0, 1), hash33(pi + vec3(1, 0, 1))),
                       	w.x),
                	w.z
				),
        		mix(
                    mix(dot(pf - vec3(0, 1, 0), hash33(pi + vec3(0, 1, 0))), 
                        dot(pf - vec3(1, 1, 0), hash33(pi + vec3(1, 1, 0))),
                       	w.x),
                   	mix(dot(pf - vec3(0, 1, 1), hash33(pi + vec3(0, 1, 1))), 
                        dot(pf - vec3(1, 1, 1), hash33(pi + vec3(1, 1, 1))),
                       	w.x),
                	w.z
				), w.y
			);
}

float r(float n) {
 	return fract(cos(n * 89.42) * 343.42);
}
vec2 r(vec2 n) {
 	return vec2(r(n.x * 23.62-300.0 + n.y * 34.35), r(n.x * 45.13 + 256.0 + n.y * 38.89)); 
}
float worley(vec2 n, float s) {
    float dis = 2.0;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 p = floor(n / s)+ vec2(x, y);
            float d = length(r(p) + vec2(x, y) - fract(n / s));
            if (dis > d) dis = d;
        }
    }
    return 1.0 - dis;
}

vec3 permute(vec3 x) {
  return mod((34.0 * x + 1.0) * x, 289.0);
}
vec3 dist(vec3 x, vec3 y, vec3 z,  bool manhattanDistance) {
  return manhattanDistance ?  abs(x) + abs(y) + abs(z) : (x * x + y * y + z * z);
}
float worley3D(vec3 P, float jitter, bool manhattanDistance) {
	float K = 0.142857142857; // 1/7
	float Ko = 0.428571428571; // 1/2-K/2
	float  K2 = 0.020408163265306; // 1/(7*7)
	float Kz = 0.166666666667; // 1/6
	float Kzo = 0.416666666667; // 1/2-1/6*2

	vec3 Pi = mod(floor(P), 289.0);
 	vec3 Pf = fract(P) - 0.5;

	vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
	vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
	vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

	vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
	vec3 p1 = permute(p + Pi.y - 1.0);
	vec3 p2 = permute(p + Pi.y);
	vec3 p3 = permute(p + Pi.y + 1.0);

	vec3 p11 = permute(p1 + Pi.z - 1.0);
	vec3 p12 = permute(p1 + Pi.z);
	vec3 p13 = permute(p1 + Pi.z + 1.0);

	vec3 p21 = permute(p2 + Pi.z - 1.0);
	vec3 p22 = permute(p2 + Pi.z);
	vec3 p23 = permute(p2 + Pi.z + 1.0);

	vec3 p31 = permute(p3 + Pi.z - 1.0);
	vec3 p32 = permute(p3 + Pi.z);
	vec3 p33 = permute(p3 + Pi.z + 1.0);

	vec3 ox11 = fract(p11*K) - Ko;
	vec3 oy11 = mod(floor(p11*K), 7.0)*K - Ko;
	vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

	vec3 ox12 = fract(p12*K) - Ko;
	vec3 oy12 = mod(floor(p12*K), 7.0)*K - Ko;
	vec3 oz12 = floor(p12*K2)*Kz - Kzo;

	vec3 ox13 = fract(p13*K) - Ko;
	vec3 oy13 = mod(floor(p13*K), 7.0)*K - Ko;
	vec3 oz13 = floor(p13*K2)*Kz - Kzo;

	vec3 ox21 = fract(p21*K) - Ko;
	vec3 oy21 = mod(floor(p21*K), 7.0)*K - Ko;
	vec3 oz21 = floor(p21*K2)*Kz - Kzo;

	vec3 ox22 = fract(p22*K) - Ko;
	vec3 oy22 = mod(floor(p22*K), 7.0)*K - Ko;
	vec3 oz22 = floor(p22*K2)*Kz - Kzo;

	vec3 ox23 = fract(p23*K) - Ko;
	vec3 oy23 = mod(floor(p23*K), 7.0)*K - Ko;
	vec3 oz23 = floor(p23*K2)*Kz - Kzo;

	vec3 ox31 = fract(p31*K) - Ko;
	vec3 oy31 = mod(floor(p31*K), 7.0)*K - Ko;
	vec3 oz31 = floor(p31*K2)*Kz - Kzo;

	vec3 ox32 = fract(p32*K) - Ko;
	vec3 oy32 = mod(floor(p32*K), 7.0)*K - Ko;
	vec3 oz32 = floor(p32*K2)*Kz - Kzo;

	vec3 ox33 = fract(p33*K) - Ko;
	vec3 oy33 = mod(floor(p33*K), 7.0)*K - Ko;
	vec3 oz33 = floor(p33*K2)*Kz - Kzo;

	vec3 dx11 = Pfx + jitter*ox11;
	vec3 dy11 = Pfy.x + jitter*oy11;
	vec3 dz11 = Pfz.x + jitter*oz11;

	vec3 dx12 = Pfx + jitter*ox12;
	vec3 dy12 = Pfy.x + jitter*oy12;
	vec3 dz12 = Pfz.y + jitter*oz12;

	vec3 dx13 = Pfx + jitter*ox13;
	vec3 dy13 = Pfy.x + jitter*oy13;
	vec3 dz13 = Pfz.z + jitter*oz13;

	vec3 dx21 = Pfx + jitter*ox21;
	vec3 dy21 = Pfy.y + jitter*oy21;
	vec3 dz21 = Pfz.x + jitter*oz21;

	vec3 dx22 = Pfx + jitter*ox22;
	vec3 dy22 = Pfy.y + jitter*oy22;
	vec3 dz22 = Pfz.y + jitter*oz22;

	vec3 dx23 = Pfx + jitter*ox23;
	vec3 dy23 = Pfy.y + jitter*oy23;
	vec3 dz23 = Pfz.z + jitter*oz23;

	vec3 dx31 = Pfx + jitter*ox31;
	vec3 dy31 = Pfy.z + jitter*oy31;
	vec3 dz31 = Pfz.x + jitter*oz31;

	vec3 dx32 = Pfx + jitter*ox32;
	vec3 dy32 = Pfy.z + jitter*oy32;
	vec3 dz32 = Pfz.y + jitter*oz32;

	vec3 dx33 = Pfx + jitter*ox33;
	vec3 dy33 = Pfy.z + jitter*oy33;
	vec3 dz33 = Pfz.z + jitter*oz33;

	vec3 d11 = dist(dx11, dy11, dz11, manhattanDistance);
	vec3 d12 = dist(dx12, dy12, dz12, manhattanDistance);
	vec3 d13 = dist(dx13, dy13, dz13, manhattanDistance);
	vec3 d21 = dist(dx21, dy21, dz21, manhattanDistance);
	vec3 d22 = dist(dx22, dy22, dz22, manhattanDistance);
	vec3 d23 = dist(dx23, dy23, dz23, manhattanDistance);
	vec3 d31 = dist(dx31, dy31, dz31, manhattanDistance);
	vec3 d32 = dist(dx32, dy32, dz32, manhattanDistance);
	vec3 d33 = dist(dx33, dy33, dz33, manhattanDistance);

	vec3 d1a = min(d11, d12);
	d12 = max(d11, d12);
	d11 = min(d1a, d13); // Smallest now not in d12 or d13
	d13 = max(d1a, d13);
	d12 = min(d12, d13); // 2nd smallest now not in d13
	vec3 d2a = min(d21, d22);
	d22 = max(d21, d22);
	d21 = min(d2a, d23); // Smallest now not in d22 or d23
	d23 = max(d2a, d23);
	d22 = min(d22, d23); // 2nd smallest now not in d23
	vec3 d3a = min(d31, d32);
	d32 = max(d31, d32);
	d31 = min(d3a, d33); // Smallest now not in d32 or d33
	d33 = max(d3a, d33);
	d32 = min(d32, d33); // 2nd smallest now not in d33
	vec3 da = min(d11, d21);
	d21 = max(d11, d21);
	d11 = min(da, d31); // Smallest now in d11
	d31 = max(da, d31); // 2nd smallest now not in d31
	d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
	d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
	d12 = min(d12, d21); // 2nd smallest now not in d21
	d12 = min(d12, d22); // nor in d22
	d12 = min(d12, d31); // nor in d31
	d12 = min(d12, d32); // nor in d32
	d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
	d11.y = min(d11.y,d12.z); // Only two more to go
	d11.y = min(d11.y,d11.z); // Done! (Phew!)

	return sqrt(d11.y) - sqrt(d11.x); // F1, F2
}

float CloudNoise(vec3 fragCoord, vec3 uv) {
	//float dis = (1.0 + PerlinNoise3(uv * 8.0)) * (1.0 + (worley(fragCoord, 32.0) + 
	// 0.5 * worley(2.0 * fragCoord, 32.0) + 0.25 * worley(4.0 * fragCoord, 32.0)));
	float dis = (1.0 + PerlinNoise3(uv * 8.0)) * 
				(1.0 + (worley3D(fragCoord, 1.0, false) + 
				0.5 * worley3D(2.0 * fragCoord, 1.0, false) + 
				0.25 * worley3D(4.0 * fragCoord, 1.0, false)));

	return dis / 4.0;
}
// -----------------------------------------------------------------------------

float SampleDensity(vec3 rayPos) {
     vec3 uvw = (rayPos - bboxMin) / 256.0; uvw *= 10.0;
     //float shapeNoise = CloudNoise(uvw * 5.0, uvw);
     //return pow(shapeNoise, 4.0) * 4.0;
     float shapeNoise = PerlinNoise3((uvw + _offset) * _scale);
     return pow(shapeNoise, _noiseMulti.x) * _noiseMulti.y;
}

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

float Attenuation(vec3 distanceVector) {
	return 1.0 / pow(length(distanceVector), 2.0);
}

float remap(float original_value, float original_min, float original_max, float new_min, float new_max) {
	return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}

// Henyey-Greenstein
float hg(float a, float g) {
	float g2 = g * g;
	return (1.0 - g2) / (4.0 * M_PI * pow(1.0 + g2 - 2.0 * g * (a), 1.5));
}

float phase(float a) {
	float blend = 0.5;
	float hgBlend = hg(a, _phaseParams.x) * (1.0 - blend) + hg(a, -_phaseParams.y) * blend;
	return _phaseParams.z + hgBlend * _phaseParams.w;
}

vec3 LightMarch(vec3 position) {
	vec3 samplePoint = position;
	vec3 dirToLight = normalize(lightPositions[0] - position);

	// If the light ray intersects with the bounding box, 
	// the exceeding part will not be calculated
	float dstInsideBox = RayBoxDst(bboxMin, bboxMax, position, 1 / dirToLight).y;
	float stepSize = dstInsideBox / _lightStep;
	float totalDensity = 0;

	// Number of light march steps
	for (int step = 0; step < _lightStep; step++) {
		position += dirToLight * stepSize;
		//totalDensity += max(0, SampleDensity(position) * stepSize);
		totalDensity += max(0, SampleDensity(position));
	}

	float cosAngle = dot(viewDir, normalize(lightPositions[0] - viewPos));
	float scatter = phase(cosAngle) * _scatterMultiply;
	
	float transmittance = exp(-totalDensity * _lightAbsorptionTowardSun);
	//transmittance = clamp(pow(transmittance, 2.0), 0, 1); // enhance shadow

	// Tone Mapping:
	// Highlight > lightColor | Midtone > mainColor | Shadow > shadowColor
	vec3 cloudColor = mix(
		_mainColor, lightColors[0] * 100, 
		clamp(
			transmittance * _midtoneOffset * 
			Attenuation(samplePoint - lightPositions[0]), 
			0.0, 1.0
		)
	);
	cloudColor = mix(
		_shadowColor, cloudColor, 
		clamp(pow(transmittance * _shadowOffset, 3.0), 0.0, 1.0)
	);
	
	//return _darknessThreshold + (1.0 - _darknessThreshold) * transmittance * cloudColor;
	return _darknessThreshold + (1.0 - _darknessThreshold) * transmittance * cloudColor * scatter;
}

vec4 CloudRayMarching(vec3 startPoint, vec3 direction, vec3 worldPos) {
	// calculate ray-bbox intersection
	float depthEyeLinear = length(worldPos - startPoint);
	vec2 intersectInfo = RayBoxDst(bboxMin, bboxMax, startPoint, 1.0 / direction);
	float dstLimit = min(depthEyeLinear - intersectInfo.x, intersectInfo.y);
	
	// start marching
	float sum = 1.0;
    vec3 lightEnergy = vec3(0);
    float stepSize = exp(_step) * _rayStep;
	if (dstLimit > 0) {
		vec3 samplePoint = startPoint + intersectInfo.x * direction;
		for (int i = 0; i < _maxMarchLoop; i++) {
			samplePoint += direction * _rayStep;
			if (_rayStep * i > dstLimit) break; // travelled dist
			else {
				float density = SampleDensity(samplePoint);
				if (density > 0) {
					vec3 lightTransmittance = LightMarch(samplePoint);
					lightEnergy += density * stepSize * sum * lightTransmittance;
					sum *= exp(-density * stepSize * _lightAbsorptionThroughCloud);
					if (sum < 0.01) break;
				}
			}
			//else sum += clamp(pow(SampleDensity(samplePoint), 8.0) * 2, 0, 1);
		}
	}

	return vec4(lightEnergy, sum);
}

void main() {
    vec3 color = texture(screenBuffer, TexCoords).rgb;
	float depth = 2 * texture(depthBuffer, TexCoords).r - 1;
	vec4 worldPos = CalculateWorldSpacePosition(depth, TexCoords * 2.0 - vec2(1.0));
	worldPos.xyz /= worldPos.w;

	// cloud ray marching
	CalculatePixelRay(TexCoords * 2.0 - vec2(1.0));
	vec4 cloud = CloudRayMarching(viewPos, viewDir, worldPos.xyz);

    FragColor = vec4(mix(vec3(cloud.r, cloud.g, cloud.b), color, cloud.a), 1.0);
}