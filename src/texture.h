#ifndef __TEXTURE__
#define __TEXTURE__

#include "global.h"
#include <fstream>
#include <sstream>
#include <stb_image.h>

class Texture
{
public:
    unsigned int ID;
    string type;
    string path;
  
    Texture(const char* imagePath, GLuint colorMode, bool gammaCorrect);
    void bind(GLuint textureSlot);
};
  

#endif /* __TEXTURE__ */
