#version 330 core
#define MAX_NR_TEXTURES 16

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D texturesDiffuse[MAX_NR_TEXTURES];
    sampler2D texturesSpecular[MAX_NR_TEXTURES];
    sampler2D textureNormal;
    float shininess;
    bool hasNormalMap;
}; 
uniform Material material;

uniform bool useNormalMaps;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;

    // also store the per-fragment normals into the gbuffer
    vec3 Normal;
    if (material.hasNormalMap && useNormalMaps) {
        // obtain normal from normal map in range [0,1]
        Normal = texture(material.textureNormal, fs_in.TexCoords).rgb;
        // transform normal vector to range [-1,1]
        Normal = Normal * 2.0 - 1.0; 
        // transform from tangent space to world space
        Normal = normalize(fs_in.TBN * Normal); 
    } else {
        Normal = normalize(fs_in.Normal);
    }
    gNormal = vec4(Normal, 1.0);

    // and the diffuse per-fragment color
    // HACK: 0 index
    gAlbedoSpec.rgb = texture(material.texturesDiffuse[0], fs_in.TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(material.texturesSpecular[0], fs_in.TexCoords).r;
}  
