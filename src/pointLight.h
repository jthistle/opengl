#ifndef __POINTLIGHT__
#define __POINTLIGHT__

#include "light.h"

class PointLight : public Light {
    const int SHADOW_SIZE = 1024;
    float _range;
    float _linear;
    float _quadratic;

    unsigned int _shadowMap;

public:
    PointLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float range, bool castsShadow);
    ~PointLight();

    float getRange() { return _range; }
    void setRange(float range);
    void bind(Shader &shader, int index, int &textureInd);
    void configureForDepthMap(Shader &shader, int framebuf);

    vector<glm::mat4> generateProjectionMatrices();
};

#endif /* __POINTLIGHT__ */
