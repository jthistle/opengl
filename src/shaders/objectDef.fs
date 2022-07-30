#version 330 core
#define PI 3.1415926535
#define MAX_NR_TEXTURES 16

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
} fs_in;

out vec4 FragColor;

struct FragData {
    vec3 FragPos;
    vec3 Albedo;
    vec3 Normal;
    float Specular;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool castsShadow;
    sampler2D shadowMap;
    mat4 lightSpaceMatrix;
};
uniform DirLight dirLight;

struct PointLight {    
    vec3 position;
    
    float linear;
    float quadratic;  
    float range;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool castsShadow;
    samplerCube shadowMap;
};  
uniform PointLight pointLights[16];
uniform int numberPointLights;

uniform vec3 viewPos;

uniform sampler2D gAlbedoSpec;
uniform sampler2D gNormal;
uniform sampler2D gTangent;
uniform sampler2D gPosition;

uniform sampler2D ssaoTexture;

uniform vec3 skyboxColor;

float ShadowCalculationDir(in vec4 fragPosLightSpace, in vec3 normal, in vec3 lightDir, in sampler2D shadowMap) {
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

float ShadowCalculationPoint(in vec3 fragPos, in PointLight light)
{
    // HACK zero index - only one light handled
    vec3 lightPos = light.position;

    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(light.shadowMap, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= light.range;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}  

vec3 CalcDirLight(in FragData data, in DirLight light, in vec3 viewDir)
{
    // Calculate direction vectors
    vec3 lightDir   = -light.direction; 
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Diffuse part
    float diff = max(dot(data.Normal, lightDir), 0.0);
    // Specular part (Blinn-Phong)
    float spec = pow(max(dot(data.Normal, halfwayDir), 0.0), 128.0); // TODO: get shininess in via gBuffer 
    
    // TODO: index 0 is hack
    vec3 ambient  = light.ambient  * data.Albedo;
    vec3 diffuse  = light.diffuse  * diff * data.Albedo;
    vec3 specular = light.specular * spec * vec3(data.Specular);

    // Shadow
    float shadow = 0.0;
    if (light.castsShadow) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(data.FragPos, 1.0); 
        shadow = ShadowCalculationDir(fragPosLightSpace, data.Normal, lightDir, light.shadowMap);  
    }

    float ssao = texture(ssaoTexture, fs_in.TexCoords).r;
    return ambient * ssao + (1.0 - shadow) * (diffuse + specular);
}

vec3 CalcPointLight(in FragData data, in PointLight light, vec3 viewDir)
{
    vec3 lightDir   = normalize(light.position - data.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Diffuse
    float diff = max(dot(data.Normal, lightDir), 0.0);
    // Blinn-Phong
    float spec = pow(max(dot(data.Normal, halfwayDir), 0.0), 128.0); // TODO shininess via gBuffer 
    // Attenuation
    float dist        = length(light.position - data.FragPos);
    float attenuation = 1.0 / (1.0 + light.linear * dist + light.quadratic * (dist * dist));    
    
    vec3 ambient  = light.ambient  * data.Albedo; 
    vec3 diffuse  = light.diffuse  * diff * data.Albedo;
    vec3 specular = light.specular * spec * vec3(data.Specular); 
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    float shadow = 0.0;
    if (light.castsShadow) {
        shadow = ShadowCalculationPoint(data.FragPos, light);
    }

    float ssao = texture(ssaoTexture, fs_in.TexCoords).r;
    return (ambient * ssao + (1.0 - shadow) * (diffuse + specular));
} 

void main()
{
    // Load data from gBuffer
    vec3 FragPos = texture(gPosition, fs_in.TexCoords).rgb;
    vec4 fullNormal = texture(gNormal, fs_in.TexCoords);
    vec3 Normal = fullNormal.rgb; 
    float isSkybox = fullNormal.a;
    vec3 Albedo = texture(gAlbedoSpec, fs_in.TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, fs_in.TexCoords).a;
    FragData data = FragData(FragPos, Albedo, Normal, Specular);

    // Skybox
    if (isSkybox == 0.0) {
        FragColor = vec4(skyboxColor, 1.0); 
        return;
    }

    // Work out useful stuff
    vec3 viewDir = normalize(viewPos - FragPos);

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(data, dirLight, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < numberPointLights; i++)
        result += CalcPointLight(data, pointLights[i], viewDir);    

    FragColor = vec4(result, 1.0);
}