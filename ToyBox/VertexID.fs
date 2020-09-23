#version 330 core
flat in int ID;

out vec4 FragColor;

void main() {
	vec4 data;

	int cnt = 0, num = ID;
	while (num > 0) {
		data[cnt++] = num % 256;
		num /= 256;
	}

	FragColor = vec4(data[0], data[1], data[2], 255 - data[3]) / 255.0;
}