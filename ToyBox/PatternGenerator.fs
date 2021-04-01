#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
//uniform sampler2D screenBuffer;
uniform int patternType;
uniform float patternScale;
uniform vec2 offset;
uniform float dim;
uniform float time;
uniform int channelSelection;

#define M_PI 3.14159265358979323846

// Bubble Pattern -----------------------------------------------------------------------------------

// Wood Pattern -------------------------------------------------------------------------------------
const float knobStrength = 2.0;
const float pixelsPerPlank = 100.0;
const float linesPerPlank = 5.5;
const float verticalScale = 6.0;
const float lineStrength = 0.8;
const float plankLength = 6.0;

float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u*u*(3.0-2.0*u);

    float res = mix(
        mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
        mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
    return res;
}

float noisetex(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
	f = f*f*(3.0-2.0*f);
	return texture( iChannel0, (p + f + 0.5)/256.0, -100.0 ).x;
}

float height(in vec2 a) {
    a = a*vec2(1.0, 1.0/verticalScale);
    return linesPerPlank * (knobStrength*noise(a) + a.x);
}

// make the lines constant width
// http://iquilezles.org/www/articles/distance/distance.htm
vec2 grad(in vec2 x) {
	vec2 h = vec2(0.05, 0.0);
	return vec2(height(x+h.xy) - height(x-h.xy),
                height(x+h.yx) - height(x-h.yx))/(2.0*h.x);
}

vec4 composit(in vec4 top, in vec4 bottom) {
    return vec4(mix(top.xyz, bottom.xyz, 1.0-top.a), 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    //animation
    fragCoord.y += iTime * 100.;
    
    vec2 pixel = fragCoord / pixelsPerPlank;
    float plank = floor(pixel.x); // unique per plank
    float start = 15.0 * rand(vec2(plank));
    float item = floor(pixel.y/plankLength + start);
    
    vec3 woodLine = vec3(90., 45., 18.)/255.;
    vec2 plankPixel = pixel + vec2(124., 11.) * plank;
    float value = height(plankPixel + item);
    vec2 gradient = grad(plankPixel + item);
    float linePos = 1.0 - smoothstep(0.0, 0.08, fract(value)/length(gradient));
    float line = floor(value); // unique per line
    float lineWeight = mix(1.0, 0.4+rand(vec2(line,plank)), 0.8);
    float lineGrain = smoothstep(-0.2, 0.9, noisetex(fragCoord));
    
    vec3 woodBase = vec3(144., 91., 49.)/255.;
    float darkness = mix(1.0, 0.5+rand(vec2(plank, item)), 0.2);
    float grain = mix(1.0, 0.5 + 0.7*noisetex(fragCoord + 12.5), 0.1);
    
    float plankGapY = step(0.0, fract(pixel.x)) * (1.0-step(0.02, fract(pixel.x)));
    float plankGapX = step(0.0, fract(pixel.y/plankLength+start)) * (1.0-step(0.02/plankLength, fract(pixel.y/plankLength+start)));
   
    // final images
    vec4 planks = vec4(0.1, 0.1, 0.1, max(plankGapY, plankGapX));
    vec4 lines = vec4(woodLine, lineStrength*lineWeight*lineGrain*linePos);
    vec4 wood = vec4(woodBase*darkness*grain, 1.0);
    
    fragColor = composit(planks, composit(lines, wood));
}

// Digital Storm ------------------------------------------------------------------------------------
float is_prime(int num) {
     if (num <= 1) return 0.0;
     if (num % 2 == 0 && num > 2) return 0.0;

     for(int i = 3; i < int(floor(sqrt(float(num)))); i+= 2) {
         if (num % i == 0) return 0.0;
     }
     return 1.0;
}


vec3 DigitalStorm(vec2 uv, vec2 offset) {
    int temp = (
        int(uv.x + time * 80.0 + offset.x)
        ^  // try & |
        int(uv.y + time * 80.0 + offset.y)
    );

    float p = is_prime(temp);
    return vec3(p / 0.4, p / 1.5, p);
}

// Truchet Pattern ----------------------------------------------------------------------------------

// Colorful Truchet ---------------------------------------------------------------------------------

// Output -------------------------------------------------------------------------------------------
void main() {
    float color = 0;
	vec2 suv = (TexCoords + offset) * patternScale;

	if (patternType == 12) { // bubble pattern
		FragColor = vec4(WhiteNoise(TexCoords, offset, noiseScale), 1.0);
		return;
	} else if (patternType == 13) { // digital storm
		color = ValueNoise(vec3(suv, dim));
	} else if (patternType == 14) { // truchet pattern

	} else if (patternType == 15) { // colorful truchet

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
