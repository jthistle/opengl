#ifndef __LIGHT__
#define __LIGHT__

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

class Light {
protected:
    float _ambient, _diffuse, _specular;
    glm::vec3 _color;

    glm::vec3 _ambientVec;
    glm::vec3 _diffuseVec;
    glm::vec3 _specularVec;

    glm::mat4 _projectionMatrix;

public:
    glm::vec3 position;

    Light(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular);

    void setColor(glm::vec3 color);
    glm::vec3 getColor() { return _color; }

    void setAmbient(float val);
    void setDiffuse(float val);
    void setSpecular(float val);

    virtual glm::mat4 generateProjectionMatrix() { return glm::mat4(1.0f); }
};

#endif /* __LIGHT__ */
