#ifndef __POINTLIGHT__
#define __POINTLIGHT__

#include <vector>
#include "light.h"

class PointLight : public Light {
    float _range;
    float _linear;
    float _quadratic;

public:
    PointLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float range);

    float getRange() { return _range; }
    void setRange(float range);
    void bind(Shader &shader, int index);

    std::vector<glm::mat4> generateProjectionMatrices();
};

#endif /* __POINTLIGHT__ */
