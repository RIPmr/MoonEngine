#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform int type;
uniform float gamma; // default 2.2
uniform sampler2D screenBuffer;
uniform sampler2D FilmLut;

float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
float A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30, W = 11.2;

vec3 Uncharted2Tonemap(vec3 x) {
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}
vec3 UnchartedStyleTonemap(vec3 texColor) {
	texColor *= 16;  // Hardcoded Exposure Adjustment

	float ExposureBias = 1.0;
	vec3 curr = Uncharted2Tonemap(ExposureBias * texColor);

	// whiteScale: Uncharted2Tonemap(vec3(W))
	vec3 color = curr / Uncharted2Tonemap(vec3(W));

	return color;
}

vec3 log10(vec3 x) {
	return log2(x) / log2(10.0);
}
vec3 Cineon(vec3 texColor) {
   texColor *= 16.0;  // Hardcoded Exposure Adjustment

   vec3 ld = vec3(0.002);
   float linReference = 0.18;
   float logReference = 444;
   float logGamma = 0.45;
      
   vec3 LogColor = log10(0.4 * texColor.rgb / linReference) / ld * logGamma + logReference;
   LogColor /= 1023.0;
   LogColor.rgb = clamp(LogColor.rgb, 0.0, 1.0);
      
   float FilmLutWidth = 256.0;
   float Padding = 0.5 / FilmLutWidth;
      
   //  apply response lookup and color grading for target display
   vec3 retColor;
   retColor.r = texture(FilmLut, vec2(mix(Padding, 1 - Padding, LogColor.r), 0.5)).r;
   retColor.g = texture(FilmLut, vec2(mix(Padding, 1 - Padding, LogColor.g), 0.5)).r;
   retColor.b = texture(FilmLut, vec2(mix(Padding, 1 - Padding, LogColor.b), 0.5)).r;

   return clamp(retColor, 0.0, 1.0);
}

vec3 ACESFilm(vec3 x) {
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 hdrColor = texture(screenBuffer, TexCoords).rgb;
	vec3 mapped = hdrColor;

	if (type == 0) { // Reinhard tone mapping
		mapped = hdrColor / (hdrColor + vec3(1.0));
	} else if (type == 1) { // Cineon converter
		mapped = Cineon(hdrColor);
	} else if (type == 2) { // ACES tone mapping
		mapped = ACESFilm(hdrColor);
	} else if (type == 3) { // Uncharted2 Style Tonemap
		mapped = UnchartedStyleTonemap(hdrColor);
	}

	if (type == 4){ // applyLUT
		// TODO
	} else {
		// Gamma correction
		mapped = pow(mapped, vec3(1.0 / gamma));
	}

    FragColor = vec4(mapped, 1.0);
}