#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 uViewPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uObjectColor;
uniform int uLightOn;

void main() {
    // Ambient
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * uLightColor;
    
    if (uLightOn == 1) {
        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uLightColor;
        
        // Specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(uViewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * uLightColor;
        
        vec3 result = (ambient + diffuse + specular) * uObjectColor;
        FragColor = vec4(result, 1.0);
    } else {
        // Samo ambient kada je svetlo ugaseno
        vec3 result = ambient * uObjectColor;
        FragColor = vec4(result, 1.0);
    }
}
