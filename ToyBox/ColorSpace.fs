#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenBuffer;
uniform float gamma;
uniform int space;
uniform int dir;

vec3 to_linear(vec3 col, int space) {
	if (space == 0) return col; // linear
	else if (space == 1){ // srgb
	
	} else if (space == 2){ // gamma
	
	} else if (space == 3){ // cineon
	
	}
}

vec3 to_srgb(vec3 linearCol) {
	
}

vec3 to_gamma(vec3 linearCol, float gamma) {
	
}

vec3 to_cineon(vec3 linearCol) {

}

vec3 to_aces_cg(vec3 linearCol) {

}

vec3 to_rec_709(vec3 linearCol) {

}

void main() {
    vec3 color_in = texture(screenBuffer, TexCoords).rgb;

	if (dir == 1) {
		color_in = to_linear(color_in, space);
	} else {
		if (space == 1){ // srgb
			color_in = to_srgb(color_in);
		} else if (space == 2){ // gamma
			color_in = to_gamma(color_in, gamma);
		} else if (space == 3){ // cineon
			color_in = to_cineon(color_in);
		}
	}

    FragColor = vec4(color_in, 1.0);
}