#ifndef __DIRECTIONALLIGHT__
#define __DIRECTIONALLIGHT__

#include "light.h"

class DirectionalLight final : public Light {

public:
    glm::vec3 direction;
    DirectionalLight(glm::vec3 color, float ambient, float diffuse, float specular, glm::vec3 direction);

    void bind(Shader& shader);
};

#endif /* __DIRECTIONALLIGHT__ */
