#include "directionalLight.h"

DirectionalLight::DirectionalLight(glm::vec3 color, float ambient, float diffuse, float specular, glm::vec3 direction_)
: Light(glm::vec3(), color, ambient, diffuse, specular) {
    direction = direction_;
}

void DirectionalLight::bind(Shader& shader) {
    shader.setVec3("dirLight.direction", direction);
    shader.setVec3("dirLight.ambient",  _ambientVec);
    shader.setVec3("dirLight.diffuse",  _diffuseVec);
    shader.setVec3("dirLight.specular", _specularVec);
}