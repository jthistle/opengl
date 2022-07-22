#ifndef __TEXTURE__
#define __TEXTURE__

#include <glad/glad.h> 
#include <stb_image.h>
  
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  

class Texture
{
public:
    unsigned int ID;
    std::string type;
    std::string path;
  
    Texture(const char* imagePath, GLuint colorMode, bool gammaCorrect);
    void bind(GLuint textureSlot);
};
  

#endif /* __TEXTURE__ */
