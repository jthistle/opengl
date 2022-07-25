#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
} vs_out;

void main()
{    
    vs_out.FragPos = aPos;
    vs_out.TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}