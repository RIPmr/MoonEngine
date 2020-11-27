#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
//uniform sampler2D screenBuffer;
uniform int noiseType;
uniform float noiseScale;
uniform vec2 offset;
uniform float dim;
uniform float time;
uniform int channelSelection;

#define M_PI 3.14159265358979323846
#define MOD3 vec3(0.1031, 0.11369, 0.13787)

// Random functions ------------------------------------------------------------------------------
float rand(float n) { return fract(sin(n) * 43758.5453123); }
float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float rand(vec2 co, float l) { return rand(vec2(rand(co), l)); }
float rand(vec2 co, float l, float t) { return rand(vec2(rand(co, l), t)); }
vec2 rand2(vec2 value) {
	vec2 pos = vec2(dot(value, vec2(127.1, 337.1)), dot(value, vec2(269.5, 183.3)));
	return fract(sin(pos) * 43758.5453123);
}

uvec2 rvec;
uint _george_marsaglia() {
	rvec.x = 36969u * (rvec.x & 65535u) + (rvec.x >> 16u);
	rvec.y = 18000u * (rvec.y & 65535u) + (rvec.y >> 16u);
	return (rvec.x << 16u) + rvec.y;
}
float rand_float() {
	return float(_george_marsaglia()) / float(0xFFFFFFFFu);
}

float mod289(float x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 perm(vec4 x) { return mod289(((x * 34.0) + 1.0) * x); }
float permute(float x) { return floor(mod(((x * 34.0) + 1.0) * x, 289.0)); }
vec4 permute(vec4 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }
vec3 permute(vec3 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }

float taylorInvSqrt(float r) { return 1.79284291400159 - 0.85373472095314 * r; }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { 
	return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); 
}
float hash(vec3 p) {
	float h = dot(p, vec3(327.73333, 10675.23857, 1736.4243589));
    return fract(sin(h) * 42387.2355837);
}
vec3 hash3(vec2 p) {
	vec3 q = vec3(dot(p, vec2(127.1,311.7)), dot(p, vec2(269.5,183.3)), dot(p, vec2(419.2,371.9)));
	return fract(sin(q) * 43758.5453);
}
vec3 hash33(vec3 p3) {
	p3 = fract(p3 * MOD3);
    p3 += dot(p3, p3.yxz + 19.19);
    return 2.0 * fract(vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x)) - 1.0;
}

vec2 fade(vec2 t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }
float fade(float t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

vec4 grad4(float j, vec4 ip){
  vec4 p, s;
  p.xyz = floor(fract(vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), vec3(1.0, 1.0, 1.0));
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 
 
  return p;
}

// White Noise -----------------------------------------------------------------------------------
vec3 WhiteNoise(vec2 uv, vec2 offset, float scale) {
	// initialize RNG
	rvec = uvec2((uv.xy + offset) * scale * 2.0);
	rvec ^= uvec2((uv.yx + offset) * scale * 2.0);

	// return a random color
	return vec3(rand_float(), rand_float(), rand_float());
}

// Generic 1,2,3 Noise ---------------------------------------------------------------------------
float GenericNoise(float p) {
	float fl = floor(p);
	float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}
 
float GenericNoise2(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
	vec2 b = floor(n);
	vec2 f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

float GenericNoise3(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);
 
    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);
 
    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);
 
    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));
 
    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);
 
    return o4.y * d.y + o4.x * (1.0 - d.y);
}

// Perlin Noise ------------------------------------------------------------------------------------
// 3D perline noise with time
float PerlinNoise4(vec2 p, float dim, float time) {
    // TODO
	return 1.0f;
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

// Simplex Noise -----------------------------------------------------------------------------------
// 2D Simplex noise
float Simplex2(vec2 v){
	const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
	vec2 i = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);
	vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz; x12.xy -= i1;
	i = mod(i, 289.0);
	vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m * m; m = m * m;
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
	vec3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

float Simplex4(vec4 v){
	const vec2 C = vec2(
		0.138196601125010504,  // (5 - sqrt(5))/20  G4
		0.309016994374947451   // (sqrt(5) - 1)/4   F4
	);

	// First corner
	vec4 i  = floor(v + dot(v, C.yyyy) );
	vec4 x0 = v -   i + dot(i, C.xxxx);
 
	// Other corners
	// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
	vec4 i0;
 
	vec3 isX = step(x0.yzw, x0.xxx);
	vec3 isYZ = step(x0.zww, x0.yyz);
	//i0.x = dot(isX, vec3(1.0));
	i0.x = isX.x + isX.y + isX.z;
	i0.yzw = 1.0 - isX;
 
	//i0.y += dot(isYZ.xy, vec2(1.0));
	i0.y += isYZ.x + isYZ.y;
	i0.zw += 1.0 - isYZ.xy;
 
	i0.z += isYZ.z;
	i0.w += 1.0 - isYZ.z;
 
	// i0 now contains the unique values 0,1,2,3 in each channel
	vec4 i3 = clamp( i0, 0.0, 1.0 );
	vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
	vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );
 
	// x0 = x0 - 0.0 + 0.0 * C 
	vec4 x1 = x0 - i1 + 1.0 * C.xxxx;
	vec4 x2 = x0 - i2 + 2.0 * C.xxxx;
	vec4 x3 = x0 - i3 + 3.0 * C.xxxx;
	vec4 x4 = x0 - 1.0 + 4.0 * C.xxxx;
 
	// Permutations
	i = mod(i, 289.0); 
	float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
	vec4 j1  = permute( permute( permute( permute (
			   i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
			 + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
			 + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
			 + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));
	// Gradients
	// (7*7*6 points uniformly over a cube, mapped onto a 4-octahedron.)
	// 7*7*6 = 294, which is close to the ring size 17*17 = 289.
	vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;
 
	vec4 p0 = grad4(j0,   ip);
	vec4 p1 = grad4(j1.x, ip);
	vec4 p2 = grad4(j1.y, ip);
	vec4 p3 = grad4(j1.z, ip);
	vec4 p4 = grad4(j1.w, ip);
 
	// Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;
	p4 *= taylorInvSqrt(dot(p4,p4));
 
	// Mix contributions from the five corners
	vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
	vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
	m0 = m0 * m0; m1 = m1 * m1;

	return 49.0 * (dot(m0 * m0, vec3(dot(p0, x0), dot(p1, x1), dot(p2, x2))) + 
	dot(m1 * m1, vec2(dot(p3, x3), dot(p4, x4)))) ;
}

float Simplex3(vec3 v){
	return Simplex4(vec4(v, 0.0));
}

// Value Noise -----------------------------------------------------------------------------------
float ValueNoise(vec3 uv) {
	float Xi = floor(uv.x);
    float Yi = floor(uv.y);
    float Zi = floor(uv.z);

    float fracX = fade(uv.x - float(Xi));
    float fracY = fade(uv.y - float(Yi));
    float fracZ = fade(uv.z - float(Zi));

    float noise11 = hash(vec3(Xi, Yi, Zi));
    float noise12 = hash(vec3(Xi + 1, Yi, Zi));
    float noise21 = hash(vec3(Xi, Yi + 1, Zi));
    float noise22 = hash(vec3(Xi + 1, Yi + 1, Zi));
    float interpolatedx1 = mix(mix(noise11, noise12, fracX), mix(noise21, noise22, fracX), fracY);

    noise11 = hash(vec3(Xi, Yi, Zi + 1));
    noise12 = hash(vec3(Xi + 1, Yi, Zi + 1));
    noise21 = hash(vec3(Xi, Yi + 1, Zi + 1));
    noise22 = hash(vec3(Xi + 1, Yi + 1, Zi + 1));
    float interpolatedx2 = mix(mix(noise11, noise12, fracX), mix(noise21, noise22, fracX), fracY);

    return mix(interpolatedx1, interpolatedx2, fracZ);
}

// Normal Noise -----------------------------------------------------------------------------------
vec3 NormalNoise(vec2 _st, float _zoom, float _speed){
    vec2 v1 = _st, v2 = _st, v3 = _st;
    float expon = pow(10.0, _zoom * 2.0);
    v1 /= 1.0 * expon; v2 /= 0.62 * expon; v3 /= 0.83 * expon;
    float n = time * _speed;
    float nr = (Simplex3(vec3(v1, n)) + Simplex3(vec3(v2, n)) + Simplex3(vec3(v3, n))) / 6.0 + 0.5;
    n = time * _speed + 1000.0;
    float ng = (Simplex3(vec3(v1, n)) + Simplex3(vec3(v2, n)) + Simplex3(vec3(v3, n))) / 6.0 + 0.5;
    return vec3(nr, ng, 0.5);
}

// Voronoi Noise -----------------------------------------------------------------------------------
float IQnoise(vec2 p, float u, float v) {
	float k = 1.0 + 63.0 * pow(1.0 - v, 6.0);

    vec2 i = floor(p);
    vec2 f = fract(p);
    
	vec2 a = vec2(0.0, 0.0);
    for (int y = -2; y <= 2; y++)
    for (int x = -2; x <= 2; x++) {
        vec2  g = vec2(x, y);
		vec3  o = hash3(i + g) * vec3(u, u, 1.0);
		vec2  d = g - f + o.xy;
		float w = pow(1.0 - smoothstep(0.0, 1.414, length(d)), k);
		a += vec2(o.z * w,w);
    }
	
    return a.x / a.y;
}

float VoronoiNoise(vec2 uv) {
	return IQnoise(24.0 * uv, 1.0, 0.0);
}

// Worley(Cell) Noise -------------------------------------------------------------------------------
float worley(vec2 uv) {
	vec2 index = floor(uv);
	vec2 pos = fract(uv);
	float d = 1.5;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vec2 p = rand2(index + vec2(i, j));
			float dist = length(p + vec2(i, j) - pos);
			d = min(dist, d);
		}
	}
	return d;
}

// Tyson polygon
vec2 worley2(vec2 uv) {
	vec2 index = floor(uv);
	vec2 pos = fract(uv);
	vec2 d = vec2(1.5, 1.5);
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			vec2 p = rand2(index + vec2(i, j));
			float dist = length(p + vec2(i, j) - pos);
			if (dist < d.x) {
				d.y = d.x;
				d.x = dist;
			} else d.y = min(dist, d.y);
		}
	}
	return d;
}

vec3 dist(vec3 x, vec3 y, vec3 z, bool manhattanDistance) {
  return manhattanDistance ?  abs(x) + abs(y) + abs(z) : (x * x + y * y + z * z);
}

vec2 worley3D(vec3 P, float jitter, bool manhattanDistance) {
	float K = 0.142857142857; // 1/7
	float Ko = 0.428571428571; // 1/2-K/2
	float K2 = 0.020408163265306; // 1/(7*7)
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
	d11.y = min(d11.y,d11.z); // Done!

	return sqrt(d11.xy); // F1, F2
}

float WorleyNoise(vec2 uv) {
	return worley(uv);
}

float WorleyNoise(vec2 uv, float dim, float time) {
	vec2 F = worley3D(vec3(uv, dim), time, false);
	return F.y - F.x;
}

float TysonPolygon(vec2 uv) {
	vec2 d = worley2(uv);
	return d.y - d.x;
}

// Worley-Perlin (Cloud) Noise ----------------------------------------------------------------------
float CloudNoise(vec2 fragCoord, float time) {
	vec2 screenCorrd = fragCoord * bufferSize;
	float dis = (1.0 + PerlinNoise3(vec3(fragCoord, time * 0.05) * 8.0)) * 
				(1.0 + (worley(screenCorrd) + 0.5 * worley(2.0 * screenCorrd) +
				0.25 * worley(4.0 * screenCorrd)));
	return dis / 4.0;
}

// FBM function -------------------------------------------------------------------------------------
float fbm(vec3 uv) {
	float f = 0, a = 1;
	for(int i = 0; i < 3; i++) {
		f += a * PerlinNoise3(uv);
		uv *= 2; a /= 2;
	}
	return f;
}

#define OCTAVES 3
float turbulence (vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 0.0;

    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
    	//snoise : 2D simplex noise function
        value += amplitude * abs(Simplex2(st));
        st *= 2.0; amplitude *= 0.5;
    }
    return value;
}

// Checker Generator --------------------------------------------------------------------------------
float Checker(vec2 uv){
	vec2 c = floor(uv) / 2.0;
	float checker = fract(c.x + c.y) * 2.0;
	return checker;
}

// Disco Light Generator ----------------------------------------------------------------------------
vec3 Disco(vec2 uv, float time){
    // get tile and center
    uv *= 16.0;
    vec2 center = floor(uv) + 0.5;
    uv = uv - center;
    
    // get max distance to border to clamp
    vec2 abuv = abs(uv);
    float border = smoothstep(0.5, 0.1, max(abuv.x, abuv.y));
    // colorize according to center and tile
    vec3 col = sin(length(center * cos(abs(center) * 49.0 + time * 0.05)) * 
			   vec3(1, 2, 3)+ time * 2.0) * 0.5 + 0.5;
    col = pow(col, vec3(10)) * border;
    
    // draw a circle
    float d = length(uv);
    float s = smoothstep(1.1, 0.1, d);
    
    // composite to final color
	vec3 result = pow(s, 12.0) * col * 100.0;
    return pow(result, vec3(1.0 / dim));
}

// Output -------------------------------------------------------------------------------------------
void main() {
    float color = 0;
	vec2 suv = (TexCoords + offset) * noiseScale;

	if (noiseType == 0) { // white noise
		FragColor = vec4(WhiteNoise(TexCoords, offset, noiseScale), 1.0);
		return;
	} else if (noiseType == 1) { // value noise
		color = ValueNoise(vec3(suv, dim));
	} else if (noiseType == 2) { // perlin noise
		color = PerlinNoise3(vec3(suv, dim));
	} else if (noiseType == 3) { // simplex noise
		color = Simplex4(vec4(suv.x, suv.y, dim, time));
	} else if (noiseType == 4) { // worley noise
		color = WorleyNoise(suv, dim, time);
	} else if (noiseType == 5) { // voronoi noise
		color = VoronoiNoise(suv);
	} else if (noiseType == 6) { // Tyson polygon
		color = TysonPolygon(suv);
	} else if (noiseType == 7) { // cloud noise
		color = CloudNoise(suv, dim);
	} else if (noiseType == 8) { // checker
		color = Checker(suv);
	} else if (noiseType == 9) { // Disco
		FragColor = vec4(Disco(suv, time), 1.0);
		return;
	}

	if (channelSelection == 0) { // rgb channel
		FragColor = vec4(vec3(color), 1.0);
	} if (channelSelection == 1) { // red channel
		FragColor = vec4(color, 0.0, 0.0, 1.0);
	} if (channelSelection == 2) { // green channel
		FragColor = vec4(0.0, color, 0.0, 1.0);
	} if (channelSelection == 3) { // blue channel
		FragColor = vec4(0.0, 0.0, color, 1.0);
	} if (channelSelection == 4) { // alpha channel
		FragColor = vec4(0.0, 0.0, 0.0, color);
	}
}
