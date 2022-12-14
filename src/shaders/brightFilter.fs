#version 330 core

in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D colorBuffer;

void main()
{    
    vec3 color = texture(colorBuffer, TexCoords).rgb;

    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}