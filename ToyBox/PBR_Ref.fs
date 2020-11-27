#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// textures
//uniform sampler2D albedoMap;
uniform sampler2D normalMap;
//uniform sampler2D metallicMap;
//uniform sampler2D roughnessMap;
//uniform sampler2D ambientMap;

// temp
uniform vec3 albedo;
uniform float metalness;
uniform float roughness;

// IBL
uniform sampler2D irradianceMap;
uniform sampler2D prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform int lightNum;
uniform vec3 lightPositions[32];
uniform vec3 lightColors[32];

uniform vec3 viewPos;

uniform float _Reflectance;
uniform float _IOR;
uniform float _Refraction;
uniform float _Fresnel;

const float PI = 3.14159265359;
const float Epsilon = 0.00001;
const float MAX_REFLECTION_LOD = 4.0;
const vec2 invAtan = vec2(0.1591, 0.3183);
// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

vec3 SampleSphericalMapLOD(vec3 dir, float mipLv) {
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
	uv *= invAtan; uv.x += 0.5; uv.y += 0.5;

	uv.y = 1.0 - uv.y;
	return textureLod(prefilterMap, uv, mipLv).rgb;
}

vec3 SampleSphericalMap(vec3 dir) {
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
	uv *= invAtan; uv.x += 0.5; uv.y += 0.5;

	uv.y = 1.0 - uv.y;
	return texture2D(irradianceMap, uv).rgb;
}

// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(float cosLh, float roughness) {
    float a2	 = pow(roughness, 4.0);
    float denom  = cosLh * cosLh * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float cosTheta, float k) {
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

float GeometrySmith(float cosLi, float cosLo, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    return GeometrySchlickGGX(cosLi, k) * GeometrySchlickGGX(cosLo, k);
}

vec3 fresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, _Fresnel);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, _Fresnel);
} 

// Refraction --------------------------------------------------------------
vec3 RefractColor(vec2 uv, vec3 n, vec3 v) {
    float fr = pow(1.0 - dot(v, n), _Fresnel) * _Reflectance;
            
    vec3 reflectDir = reflect(-v, n);
    vec3 refractDir = refract(-v, n, _IOR - 1.0);

    vec3 reflectColor = SampleSphericalMapLOD(normalize(reflectDir), roughness * MAX_REFLECTION_LOD);
    vec3 refractColor = SampleSphericalMapLOD(normalize(refractDir), roughness * MAX_REFLECTION_LOD);
            
    return reflectColor * fr + refractColor;
}

void main() {		
    // material properties
    //vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    //float metallic = texture(metallicMap, TexCoords).r;
    //float roughness = texture(roughnessMap, TexCoords).r;
    //float ao = texture(ambientMap, TexCoords).r;
       
    // input lighting data
    vec3 Lo = normalize(viewPos - WorldPos);
    //vec3 N = GetNormalFromMap();
    vec3 N = normalize(Normal);
	// Angle between surface normal and outgoing light direction
	float cosLo = max(dot(N, Lo), 0.0);
	// Specular reflection vector
	vec3 Lr = 2.0 * cosLo * N - Lo;

    // calculate reflectance at normal incidence; 
	// if dia-electric (like plastic) use F0 of 0.04 and 
	// if it's a metal, use the albedo color as F0 (metallic workflow) 
    vec3 F0 = mix(Fdielectric, albedo, metalness);

    // reflectance equation
    vec3 directLighting = vec3(0.0);
    for(int i = 0; i < lightNum; ++i) {
        // calculate per-light radiance
        vec3 Li = normalize(lightPositions[i] - WorldPos);
		// Half-vector between Li and Lo
        vec3 Lh = normalize(Li + Lo);

        float distance		= length(lightPositions[i] - WorldPos);
        float attenuation	= 1.0 / (distance * distance);
        vec3  Lradiance		= lightColors[i] * attenuation;
		
		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(dot(N, Li), 0.0);
		float cosLh = max(dot(N, Lh), 0.0);

        // Cook-Torrance BRDF
		// Calculate Fresnel term for direct lighting
        vec3  F = fresnelSchlick(F0, max(dot(Lh, Lo), 0.0));
		// Calculate normal distribution for specular BRDF
        float D = DistributionGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF
        float G = GeometrySmith(cosLi, cosLo, roughness); 

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);
		
		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		vec3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

	vec3 ambientLighting;
	
	// Sample diffuse irradiance at normal direction
    vec3 irradiance = SampleSphericalMap(N);
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlick(F0, cosLo);
    //vec3 F = fresnelSchlickRoughness(F0, cosLo, roughness);
    
	// Get diffuse contribution factor (as with direct lighting)
	vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);
	
	// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
	vec3 diffuseIBL = kd * albedo * irradiance;

	// Sample pre-filtered specular reflection environment at correct mipmap level.
	vec3 specularIrradiance = SampleSphericalMapLOD(Lr, roughness * MAX_REFLECTION_LOD);

	// Split-sum approximation factors for Cook-Torrance specular BRDF.
	vec2 specularBRDF = texture(brdfLUT, vec2(cosLo, roughness)).rg;

	// Total specular IBL contribution.
	vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

	// Total ambient lighting contribution.
	ambientLighting = diffuseIBL + specularIBL * _Reflectance;
	
	// Calculate refraction
	vec3 RefractionColor = RefractColor(TexCoords, N, Lo);

	// Mix output
    FragColor = vec4(mix(directLighting + ambientLighting, RefractionColor, _Refraction), 1.0);
}
