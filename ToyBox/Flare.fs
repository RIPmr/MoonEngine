#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenBuffer;
uniform float threshold;
uniform float intensity;
uniform float stretch;
uniform float brightness;
uniform vec3 tint;

float GetLight(vec2 uv){
	vec3 color = texture(screenBuffer, uv).rgb;
	return pow((color.r + color.g + color.b) / 3.0, threshold) * (1.0 - threshold);
}

vec3 flares(sampler2D tex, vec2 uv, float _threshold, float intensity, float stretch, float brightness) {
    _threshold = 1.0 - _threshold;
    
    float hdr = floor(_threshold + GetLight(uv));
    //hdr = vec3(floor(_threshold + pow(hdr.r, 1.0)));

    float d = intensity;
    float c = intensity * stretch;
    
    // horizontal
    for (float i = c; i > -1.0; i--) {
		float texL = GetLight(uv + vec2(i / d, 0.0));
		float texR = GetLight(uv - vec2(i / d, 0.0));
        hdr += floor(_threshold + pow(max(texL, texR), 4.0)) * (1.0 - i / c);
    }
    
    // vertical
    //for (float i = c / 2.0; i > -1.0; i--) {
    //    float texU = texture(tex, uv + vec2(0.0, i / d)).r;
    //    float texD = texture(tex, uv - vec2(0.0, i / d)).r;
    //    hdr += floor(_threshold + pow(max(texU, texD), 40.0)) * (1.0 - i / c) * 0.25;
    //}

	return vec3(hdr) * tint * brightness / threshold;
}

void main() {
    vec3 color = texture(screenBuffer, TexCoords).rgb;
	
    color += flares(screenBuffer, TexCoords, 0.9, intensity, stretch, brightness / 100.0);

    FragColor = vec4(color, 1.0);
}