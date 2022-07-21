#version 330 core

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D quadTexture;

out vec4 FragColor;

void main()
{    
    FragColor = texture(quadTexture, TexCoords);
}