#version 330 core
#define PI 3.1415926535
#define MAX_NR_TEXTURES 16

struct Material {
    sampler2D texturesDiffuse[MAX_NR_TEXTURES];
    sampler2D texturesSpecular[MAX_NR_TEXTURES];
    float shininess;
}; 
uniform Material material;

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform PointLight pointLights[16];
uniform int numberPointLights;

uniform vec3 viewPos;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    // Calculate direction vectors
    vec3 lightDir   = -light.direction; 
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Diffuse part
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular part (Blinn-Phong)
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // TODO: index 0 is hack
    vec3 ambient  = light.ambient  * vec3(texture(material.texturesDiffuse[0], TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texturesDiffuse[0], TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texturesSpecular[0], TexCoords));

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir   = normalize(light.position - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // Blinn-Phong
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // Attenuation
    float dist        = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + light.linear * dist + light.quadratic * (dist * dist));    
    
    // TODO: index 0 hack
    vec3 ambient  = light.ambient  * vec3(texture(material.texturesDiffuse[0], TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texturesDiffuse[0], TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texturesSpecular[0], TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < numberPointLights; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    

    FragColor = vec4(result, 1.0);
}