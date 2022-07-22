#version 330 core
#define PI 3.1415926535
#define MAX_NR_TEXTURES 16

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct Material {
    sampler2D texturesDiffuse[MAX_NR_TEXTURES];
    sampler2D texturesSpecular[MAX_NR_TEXTURES];
    sampler2D textureNormal;
    float shininess;
    bool hasNormalMap;
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
uniform sampler2D shadowMap;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
     // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

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
    vec3 ambient  = light.ambient  * vec3(texture(material.texturesDiffuse[0], fs_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texturesDiffuse[0], fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texturesSpecular[0], fs_in.TexCoords));

    // Shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);  

    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir   = normalize(light.position - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // Blinn-Phong
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // Attenuation
    float dist        = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + light.linear * dist + light.quadratic * (dist * dist));    
    
    // TODO: index 0 hack
    vec3 ambient  = light.ambient  * vec3(texture(material.texturesDiffuse[0], fs_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texturesDiffuse[0], fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texturesSpecular[0], fs_in.TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main()
{
    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    if (material.hasNormalMap) {
        // obtain normal from normal map in range [0,1]
        norm = texture(material.textureNormal, fs_in.TexCoords).rgb;
        // transform normal vector to range [-1,1]
        norm = normalize(norm * 2.0 - 1.0);   
    }

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < numberPointLights; i++)
        result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir);    

    FragColor = vec4(result, 1.0);
}