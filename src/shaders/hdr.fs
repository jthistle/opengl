#version 330 core

in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D colorBuffer;

void main()
{    
    const float gamma = 2.2;
    vec3 hdrColor = texture(colorBuffer, TexCoords).rgb;
  
    // reinhard tone mapping
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    float exposure = 0.1;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
    
    // pass-through
    // FragColor = vec4(texture(colorBuffer, TexCoords).rgb, 1.0);
}