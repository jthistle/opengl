#include <glm/gtc/matrix_transform.hpp>
#include "pointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float range, bool castsShadow)
: Light(position, color, ambient, diffuse, specular, castsShadow) {
    setRange(range);
    
    if (castsShadow) {
        glGenTextures(1, &_shadowMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _shadowMap);
        for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                            SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);  
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Unbind to prevent render failure
        // See https://stackoverflow.com/questions/73124083/simply-generating-a-cubemap-leads-to-a-black-screen/73127330#73127330
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
}

PointLight::~PointLight() {
    if (_castsShadow) {
        unsigned int textures[] = { _shadowMap };
        glDeleteTextures(1, textures);
    }
}

void PointLight::setRange(float range) {
    _range = range;
    // Based off sample value of range 13 giving 0.35, 0.44 coeffs.
    _linear = (13.0f / range) * 0.35f;
    _quadratic = pow(13.0f / range, 2.0f) * 0.44f;
}

void PointLight::bind(Shader &shader, int i, int &textureInd) {
    shader.setVec3("pointLights[" + std::to_string(i) + "].position", position);
    shader.setFloat("pointLights[" + std::to_string(i) + "].linear", _linear);
    shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", _quadratic);
    shader.setVec3("pointLights[" + std::to_string(i) + "].ambient", _ambientVec);
    shader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", _diffuseVec);
    shader.setVec3("pointLights[" + std::to_string(i) + "].specular", _specularVec);
    shader.setFloat("pointLights[" + std::to_string(i) + "].range", _range);
    shader.setBool("pointLights[" + std::to_string(i) + "].castsShadow", _castsShadow);

    if (_castsShadow) {
        glActiveTexture(GL_TEXTURE0 + textureInd);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _shadowMap);
        shader.setInt("pointLights[" + std::to_string(i) + "].shadowMap", textureInd);
        textureInd++;
    } 
}

void PointLight::configureForDepthMap(Shader &shader, int framebuf) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    shader.use();
    shader.setVec3("lightPos", position);
    shader.setFloat("farPlane", _range);
    auto matrices = generateProjectionMatrices();
    for (int i = 0; i < matrices.size(); i++) {
        shader.setMat4("shadowMatrices[" + std::to_string(i) + "]", matrices[i]);
    }
    glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
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