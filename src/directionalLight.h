#ifndef __DIRECTIONALLIGHT__
#define __DIRECTIONALLIGHT__

#include "light.h"

class DirectionalLight final : public Light {
    const int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int _shadowMap;

public:
    glm::vec3 direction;
    DirectionalLight(glm::vec3 color, float ambient, float diffuse, float specular, glm::vec3 direction);

    void configureForDepthMap(Shader &shader, unsigned int framebuf);
    void bind(Shader& shader, int &textureInd);
    glm::mat4 generateProjectionMatrix();
};

#endif /* __DIRECTIONALLIGHT__ */
