#include "light.h"

Light::Light(glm::vec3 position_, glm::vec3 color, float ambient, float diffuse, float specular) {
    position = position_;
    _ambient = ambient;
    _diffuse = diffuse;
    _specular = specular;

    setColor(color);
}

void Light::setColor(glm::vec3 color) {
    _color = color;
    _ambientVec = _ambient * color;
    _diffuseVec = _diffuse * color;
    _specularVec = _specular * color;
}

void Light::setAmbient(float val) {
    _ambient = val;
    _ambientVec = _color * val;
}

void Light::setDiffuse(float val) {
    _diffuse = val;
    _diffuseVec = _color * val;
}

void Light::setSpecular(float val) {
    _specular = val;
    _specularVec = _color * val;
}
