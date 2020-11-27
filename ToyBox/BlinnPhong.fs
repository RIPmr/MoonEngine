#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float specularLV;

void main(){
	//ambient
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);

	// difference with Phong model ----------------------
    vec3 halfDir = normalize(viewDir + lightDir); 
    float spec = pow(max(dot(halfDir, norm), 0.0), specularLV);
	// --------------------------------------------------

    vec3 specular = specularStrength * lightColor * clamp(spec, 0.0, 1.0);
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}
