#ifndef __LIGHT__
#define __LIGHT__

#include "global.h"

#include "shader.h"

class Light {
protected:
    float _ambient, _diffuse, _specular;
    glm::vec3 _color;
    bool _castsShadow;

    glm::vec3 _ambientVec;
    glm::vec3 _diffuseVec;
    glm::vec3 _specularVec;

    glm::mat4 _projectionMatrix;

public:
    glm::vec3 position;

    Light(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, bool castsShadow);

    void setColor(glm::vec3 color);
    glm::vec3 getColor() { return _color; }
    bool getCastsShadow() { return _castsShadow; }

    void setAmbient(float val);
    void setDiffuse(float val);
    void setSpecular(float val);
};

#endif /* __LIGHT__ */
