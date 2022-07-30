#version 330 core
#define KERNEL_SIZE 64
#define RADIUS 0.2
#define BIAS 0.025

out float FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[KERNEL_SIZE];
uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenRes;

// tile noise texture over screen, based on screen dimensions divided by noise size
vec2 noiseScale = screenRes / 4.0;

void main()
{
    vec3 fragPos   = (view * vec4(texture(gPosition, TexCoords).xyz, 1.0)).xyz;
    vec3 normal    = normalize(transpose(inverse(mat3(view))) * texture(gNormal, TexCoords).rgb);
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;  

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);  

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * RADIUS; 
        
        vec4 offset = vec4(samplePos, 1.0);
        offset      = projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0   

        float sampleDepth = (view * vec4(texture(gPosition, offset.xy).xyz, 1.0)).z; 
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;
    }  

    occlusion = 1.0 - (occlusion / KERNEL_SIZE);
    FragColor = occlusion;
}