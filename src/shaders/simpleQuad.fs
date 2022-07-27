#version 330 core

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D quadTexture;

out vec4 FragColor;

#define near 0.1
#define far 100.0

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{    
    float depth = LinearizeDepth(texture(quadTexture, TexCoords).r) / far; // divide by far for demonstration
    FragColor = vec4(vec3(depth), 1.0);
}