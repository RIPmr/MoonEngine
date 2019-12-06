#version 330 core

uniform bool isSelected;
out vec4 FragColor;

void main() {
    FragColor = isSelected ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 0.0, 1.0, 1.0);
}