#include "pointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float range)
: Light(position, color, ambient, diffuse, specular) {
    setRange(range);
}

void PointLight::setRange(float range) {
    _range = range;
    // Based off sample value of range 13 giving 0.35, 0.44 coeffs.
    _linear = (13.0f / range) * 0.35f;
    _quadratic = pow(13.0f / range, 2.0f) * 0.44f;
}

void PointLight::bind(Shader& shader, int i) {
    shader.setVec3("pointLights[" + std::to_string(i) + "].position", position);
    shader.setFloat("pointLights[" + std::to_string(i) + "].linear", _linear);
    shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", _quadratic);
    shader.setVec3("pointLights[" + std::to_string(i) + "].ambient", _ambientVec);
    shader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", _diffuseVec);
    shader.setVec3("pointLights[" + std::to_string(i) + "].specular", _specularVec);
}
