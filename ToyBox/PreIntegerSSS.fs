#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform int specType;

uniform vec3 tintColor;
uniform vec3 specColor;
uniform vec3 scatColor;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D scatterMap;
uniform sampler2D SSSLUTMap;
uniform sampler2D kelemenLUTMap;

uniform float _roughness;
uniform float _specMulti;
uniform float _curveFactor;

uniform float _distortion;
uniform float _scatMulti;
uniform float _scatScale;

void main() {
   FragColor = vec4(vec3(0.5), 1.0);
}
