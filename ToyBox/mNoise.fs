#ifdef NOISE_DIM3
    #define VECD vec3
#endif

#ifdef NOISE_DIM4
    #define VECD vec4
    uniform float time;
#endif


#define one 0.00390625 //1.0 / 256.0;
uniform sampler2D permSampler2d  ;
uniform sampler2D permGradSampler;
uniform sampler2D permSampler;
uniform sampler2D gradSampler;



vec3 SmoothCurve(vec3 t) {
    return t*t*( 3.0 - 2.0 * t);
    //return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

vec4 SmoothCurve(vec4 t) {
    return t*t*( 3.0 - 2.0 * t);
    //return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}


/////////////////////////////
// 2D permutation table
/////////////////////////////
vec4 perm2d(vec2 p) {
	return texture2D(permSampler2d, p);
}


/////////////////////////////
// 1D permutation table
/////////////////////////////
float perm(float x) {
	return texture2D(permSampler, vec2(x, 0.5)).x;
}

/////////////////////////////
// 3D/4D permutation table
/////////////////////////////
float grad(float x, vec4 p) {
	return dot(texture2D(gradSampler, vec2(x, 0.5)), p);
}



/////////////////////////////
// 4D permutated gradient table
/////////////////////////////
float gradperm(float x, vec4 p) {
	return dot(texture2D(permGradSampler, vec2(x, 0.5)), p);
}


/////////////////////////////
// 3D permutated gradient table
/////////////////////////////
float gradperm(float x, vec3 p) {
	return dot(texture2D(permGradSampler, vec2(x, 0.5)).xyz, p);
}

/*
// calculate gradient of noise (expensive!)
vec3 inoiseGradient(vec3 p, float d) {
	float f0 = inoise(p);
	float fx = inoise(p + vec3(d, 0, 0));	
	float fy = inoise(p + vec3(0, d, 0));
	float fz = inoise(p + vec3(0, 0, d));
	return vec3(fx - f0, fy - f0, fz - f0) / d;
}
*/


//////////////////
// 3D better
//////////////////

// optimized version
float mNoise(vec3 pt) {
	vec3 P = mod(floor(pt), 256.0) * one;	// FIND UNIT CUBE THAT CONTAINS POINT
  	vec3 p = pt - floor(pt);                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	//const float one = 1.0 / 256.0;
	
    // HASH COORDINATES OF THE 8 CUBE CORNERS
	vec4 AA = perm2d(P.xy) + P.z;

    vec3 f = SmoothCurve(p);                 // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.

    return mix( 
               mix( 
                   mix( gradperm(AA.x, p ),  
                        gradperm(AA.z, p + vec3(-1,  0, 0)), f.x),
                   mix( gradperm(AA.y, p + vec3( 0, -1, 0)),
                        gradperm(AA.w, p + vec3(-1, -1, 0)), f.x), 
               f.y),                             
               mix( 
                   mix( gradperm(AA.x+one, p + vec3( 0,  0, -1) ),
                        gradperm(AA.z+one, p + vec3(-1,  0, -1) ), f.x),
                   mix( gradperm(AA.y+one, p + vec3( 0, -1, -1) ),
                        gradperm(AA.w+one, p + vec3(-1, -1, -1) ), f.x), 
               f.y), 
           f.z);
 

}

//////////////////
// 4D better
//////////////////

float mNoise(vec4 pt) {
	vec4 P = mod(floor(pt), 256.0) * one;	// FIND UNIT CUBE THAT CONTAINS POINT
  	vec4 p = pt - floor(pt);                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.

    vec4 AA = perm2d(P.xy) + P.z;
    
	float AAA = perm(AA.x)+P.w, AAB = perm(AA.x+one)+P.w;
    float ABA = perm(AA.y)+P.w, ABB = perm(AA.y+one)+P.w;
    float BAA = perm(AA.z)+P.w, BAB = perm(AA.z+one)+P.w;
    float BBA = perm(AA.w)+P.w, BBB = perm(AA.w+one)+P.w;


    vec4 f = SmoothCurve(p);                 // COMPUTE FADE CURVES FOR EACH OF X,Y,Z,W


  	return mix(
             mix( mix( mix( gradperm(AAA, p ), 
                            gradperm(BAA, p + vec4(-1, 0, 0, 0) ), f.x), 
                       mix( gradperm(ABA, p + vec4(0, -1, 0, 0) ), 
                            gradperm(BBA, p + vec4(-1, -1, 0, 0) ), f.x), f.y),                              
                  mix( mix( gradperm(AAB, p + vec4(0, 0, -1, 0) ),
                            gradperm(BAB, p + vec4(-1, 0, -1, 0) ), f.x),
                       mix( gradperm(ABB, p + vec4(0, -1, -1, 0) ),
                            gradperm(BBB, p + vec4(-1, -1, -1, 0) ), f.x), f.y), f.z),
                                
             mix( mix( mix( gradperm(AAA+one, p + vec4(0, 0, 0, -1)),
                            gradperm(BAA+one, p + vec4(-1, 0, 0, -1) ), f.x),
                       mix( gradperm(ABA+one, p + vec4(0, -1, 0, -1) ),
                            gradperm(BBA+one, p + vec4(-1, -1, 0, -1) ), f.x), f.y),
                                              
                  mix( mix( gradperm(AAB+one, p + vec4(0, 0, -1, -1) ),
                            gradperm(BAB+one, p + vec4(-1, 0, -1, -1) ), f.x),
                       mix( gradperm(ABB+one, p + vec4(0, -1, -1, -1) ),
                            gradperm(BBB+one, p + vec4(-1, -1, -1, -1) ), f.x), f.y), f.z), f.w
			);


}


// fractal sum
float fBm(VECD p, int octaves, float lacunarity = 2.0, float gain = 0.5) {
	float freq = 1.0, amp = 0.5;
	float sum = 0.0;	
	for(int i=0; i<octaves; i++) {
		sum += mNoise(p*freq)*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float turbulence(VECD p, int octaves, float lacunarity = 2.0, float gain = 0.5) {
	float sum = 0.0;
	float freq = 1.0, amp = 1.0;
	for(int i=0; i<octaves; i++) {
		//sum += abs(inoise(vec4(p.x, p.y,0.0,p.z)*freq))*amp;
        sum += abs(mNoise(p*freq))*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

// Ridged multifractal
// See "Texturing & Modeling, A Procedural Approach", Chapter 12
float ridge(float h, float offset) {
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(VECD p, int octaves, float lacunarity = 2.0, float gain = 0.5, float offset = 1.0) {
	float sum = 0.0;
	float freq = 1.0, amp = 0.5;
	float prev = 1.0;
	for(int i=0; i<octaves; i++) {
		float n = ridge(mNoise(p*freq), offset);
		sum += n*amp*prev;
		prev = n;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float fabsnoise(VECD p, int octaves, float lacunarity = 2.0, float gain = 0.5) {

  float amp = 1.0;
  float sum = 0.0;
  float freq= 1.0;

  for(int i=0; i<octaves; i++) {
    sum += abs(mNoise(p*freq)) * amp;
    amp *= gain;
    freq*= lacunarity;
  }

  return sum;

}

float marble(VECD p, int octaves) {

    return sin((p.x*7.0 + turbulence(p, octaves)*9.0) * 3.14159265);

}

float clouds(VECD p, int octaves) {

    return 2.0*turbulence(p, octaves) - 0.55;
}


void main() {
    vec4 c = gl_TexCoord[0];

    //c = vec3(gl_TexCoord[0].xy, gl_TexCoord[0].z + sin(gl_TexCoord[0].z+gl_TexCoord[0].x*3.1415) * cos(gl_TexCoord[0].z+gl_TexCoord[0].y*3.1415) * .25); 

    //vec3 v = c.xyz;
    vec4 v = vec4(c.xyz*2.0, time);
    //vec4 v = c;

    float col = (clouds(v, 4) + 1.0) * .5;
    gl_FragColor = vec4(col, col, col, col);

}