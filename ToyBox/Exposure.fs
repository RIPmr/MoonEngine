#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform float exposure;
uniform float highlight;
uniform float contrast;
uniform sampler2D screenBuffer;

void main() {
    vec3 hdrColor = texture(screenBuffer, TexCoords).rgb;

	// exposure
	vec3 mapped = hdrColor * exposure;

	// saturation
	//float luminance = 0.2125 * hdrColor.r + 0.7154 * hdrColor.g + 0.0721 * hdrColor.b;
    //vec3 luminanceColor = vec3(luminance, luminance, luminance);
    //mapped = mix(luminanceColor, mapped, saturation);

	// highlight
	if ((mapped.r + mapped.g + mapped.b) / 3.0 > 1.0) {
		mapped = mix(normalize(mapped), mapped, highlight);
	}

	// contrast
    vec3 avgColor = vec3(0.5, 0.5, 0.5);
    mapped = mix(avgColor, mapped, contrast);

    FragColor = vec4(mapped, 1.0);
}