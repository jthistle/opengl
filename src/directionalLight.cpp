#include <glm/gtc/matrix_transform.hpp>
#include "directionalLight.h"

DirectionalLight::DirectionalLight(glm::vec3 color, float ambient, float diffuse, float specular, glm::vec3 direction_)
: Light(glm::vec3(), color, ambient, diffuse, specular) {
    direction = direction_;

    // Generate projection matrix
    float near_plane = 1.0f, far_plane = 10.0f;
    glm::vec3 position = glm::vec3(2.0f, 4.0f, 2.0f);
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 
    glm::mat4 lightView = glm::lookAt(position, 
                                    position + direction,
                                    glm::vec3( 0.0f, 1.0f,  0.0f));  
    _projectionMatrix = lightProjection * lightView;
}

void DirectionalLight::bind(Shader& shader) {
    shader.setVec3("dirLight.direction", direction);
    shader.setVec3("dirLight.ambient",  _ambientVec);
    shader.setVec3("dirLight.diffuse",  _diffuseVec);
    shader.setVec3("dirLight.specular", _specularVec);
}
