#include "texture.h"

Texture::Texture(const char* imagePath, GLuint colorMode)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // stbi config
    // image coords are different to gl coords, so flip image
    stbi_set_flip_vertically_on_load(true);  

    // load and generate the texture
    int width, height, nrChannels;
    std::cout << "Loading texture at" << imagePath << std::endl;
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture at" << imagePath << std::endl;
    }
    stbi_image_free(data); 
}

void Texture::bind(GLuint textureSlot)
{
    glActiveTexture(textureSlot);
    glBindTexture(GL_TEXTURE_2D, ID);
}