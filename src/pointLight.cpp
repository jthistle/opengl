#include <glm/gtc/matrix_transform.hpp>
#include "pointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float range)
: Light(position, color, ambient, diffuse, specular, true) {
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


std::vector<glm::mat4> PointLight::generateProjectionMatrices() {
    float aspect = 1.0f; // TODO not hardcode 
    float near = 1.0f;
    float far = _range;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far); 
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(position, position + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(position, position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(position, position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(position, position + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(position, position + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

    return shadowTransforms;
}