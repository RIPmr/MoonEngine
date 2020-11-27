#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 bufferSize;
uniform sampler2D screenBuffer;

uniform bool debug;

uniform float _threshold;
uniform float _weight;
uniform float _exposure;

// precomputed kernal, sigma = 7.0
const float kernel2[5] = float[5]( 
	0.0547121, 0.0564128, 0.0569914, 0.0564128, 0.0547121
);
const float kernel5[11] = float[11]( 
	0.0441591, 0.0484066, 0.0519907, 0.0547121, 0.0564128, 0.0569914, 0.0564128, 0.0547121, 0.0519907, 0.0484066, 0.0441591
);
const float kernel10[21] = float[21]( 
	0.0205424, 0.0249374, 0.0296612, 0.034567, 0.0394705, 0.0441591, 0.0484066, 0.0519907, 0.0547121, 0.0564128, 0.0569914, 0.0564128, 0.0547121, 0.0519907, 0.0484066, 0.0441591, 0.0394705, 0.034567, 0.0296612, 0.0249374, 0.0205424
);
const float kernel20[41] = float[41]( 
	0.000962009, 0.00143222, 0.00208919, 0.00298596, 0.00418144, 0.00573726, 0.00771295, 0.0101595, 0.0131118, 0.0165802, 0.0205424, 0.0249374, 0.0296612, 0.034567, 0.0394705, 0.0441591, 0.0484066, 0.0519907, 0.0547121, 0.0564128, 0.0569914, 0.0564128, 0.0547121, 0.0519907, 0.0484066, 0.0441591, 0.0394705, 0.034567, 0.0296612, 0.0249374, 0.0205424, 0.0165802, 0.0131118, 0.0101595, 0.00771295, 0.00573726, 0.00418144, 0.00298596, 0.00208919, 0.00143222, 0.000962009
);
const float kernel40[81] = float[81]( 
	4.62687e-09, 1.03605e-08, 2.27307e-08, 4.88632e-08, 1.02917e-07, 2.12387e-07, 4.29445e-07, 8.50791e-07, 1.65149e-06, 3.14098e-06, 5.85317e-06, 1.0687e-05, 1.91185e-05, 3.35112e-05, 5.75525e-05, 9.68444e-05, 0.000159669, 0.000257932, 0.00040825, 0.000633118, 0.000962009, 0.00143222, 0.00208919, 0.00298596, 0.00418144, 0.00573726, 0.00771295, 0.0101595, 0.0131118, 0.0165802, 0.0205424, 0.0249374, 0.0296612, 0.034567, 0.0394705, 0.0441591, 0.0484066, 0.0519907, 0.0547121, 0.0564128, 0.0569914, 0.0564128, 0.0547121, 0.0519907, 0.0484066, 0.0441591, 0.0394705, 0.034567, 0.0296612, 0.0249374, 0.0205424, 0.0165802, 0.0131118, 0.0101595, 0.00771295, 0.00573726, 0.00418144, 0.00298596, 0.00208919, 0.00143222, 0.000962009, 0.000633118, 0.00040825, 0.000257932, 0.000159669, 9.68444e-05, 5.75525e-05, 3.35112e-05, 1.91185e-05, 1.0687e-05, 5.85317e-06, 3.14098e-06, 1.65149e-06, 8.50791e-07, 4.29445e-07, 2.12387e-07, 1.02917e-07, 4.88632e-08, 2.27307e-08, 1.03605e-08, 4.62687e-09
);

// precomputed normalization factor (as the gaussian has been clamped)
const float Z2  = 0.0504134;
const float Z5  = 0.27478;
const float Z10 = 0.716002;
const float Z20 = 0.991327;
const float Z40 = 0.999988;

vec3 GetLight(vec2 uv){
	vec3 color = texture(screenBuffer, uv).rgb;
	if ((color.r + color.g + color.b) / 3.0 >= _threshold) return color;
	else return vec3(0.0);
}

vec3 GaussianBlur(int kSize){
	vec3 final_colour = vec3(0.0);

	for (int i = -kSize; i <= kSize; ++i) {
		for (int j = -kSize; j <= kSize; ++j) {
			float k = 1.0;
			if (kSize == 2)		  k = kernel2[kSize + j]  * kernel2[kSize + i];
			else if (kSize == 5)  k = kernel5[kSize + j]  * kernel5[kSize + i];
			else if (kSize == 10) k = kernel10[kSize + j] * kernel10[kSize + i];
			else if (kSize == 20) k = kernel20[kSize + j] * kernel20[kSize + i];
			else if (kSize == 40) k = kernel40[kSize + j] * kernel40[kSize + i];
			final_colour += k * GetLight(TexCoords + vec2(float(i), float(j)) / bufferSize);
		}
	}

	return final_colour / (kSize == 2 ? Z2 : (kSize == 5 ? Z5 : (kSize == 10 ? Z10 : (kSize == 20 ? Z20 : Z40))));
}

void main() {
	if (debug) {
		FragColor = vec4(GetLight(TexCoords), 1.0);
	} else {
		vec3 color = texture(screenBuffer, TexCoords).rgb;

		vec3 light = GaussianBlur(2);
		light += GaussianBlur(5) * 1.3;
		light += GaussianBlur(10) * 1.6;
		light += GaussianBlur(20) * 1.9;
		//light += GaussianBlur(40) * 2.2;

		FragColor = vec4(color + pow(light, vec3(_exposure)) * _weight, 1.0);
	}
}
