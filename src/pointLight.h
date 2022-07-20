#ifndef __POINTLIGHT__
#define __POINTLIGHT__

#include "light.h"

class PointLight : public Light {
    float _linear;
    float _quadratic;

public:
    PointLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float range);

    void setRange(float range);
    void bind(Shader &shader, int index);
};

#endif /* __POINTLIGHT__ */
