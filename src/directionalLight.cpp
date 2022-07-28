#include <glm/gtc/matrix_transform.hpp>
#include "directionalLight.h"

DirectionalLight::DirectionalLight(glm::vec3 color, float ambient, float diffuse, float specular, glm::vec3 direction, bool castsShadow)
: Light(glm::vec3(), color, ambient, diffuse, specular, castsShadow) {
    this->direction = direction;
    
    if (_castsShadow) {
        // Generate a shadow map texture
        glGenTextures(1, &_shadowMap);
        glBindTexture(GL_TEXTURE_2D, _shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        // Clamp so that areas outside map are not in shadow
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
    }
}

DirectionalLight::~DirectionalLight() {
    if (_castsShadow) {
        unsigned int textures[] = { _shadowMap };
        glDeleteTextures(1, textures);
    }
}

void DirectionalLight::configureForDepthMap(Shader &shader, unsigned int framebuf) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setMat4("lightSpaceMatrix", generateProjectionMatrix());
}

void DirectionalLight::bind(Shader& shader, int &textureInd) {
    shader.setVec3("dirLight.direction", direction);
    shader.setVec3("dirLight.ambient",  _ambientVec);
    shader.setVec3("dirLight.diffuse",  _diffuseVec);
    shader.setVec3("dirLight.specular", _specularVec);
    shader.setBool("dirLight.castsShadow", _castsShadow);

    if (_castsShadow) {
        glActiveTexture(GL_TEXTURE0 + textureInd);
        glBindTexture(GL_TEXTURE_2D, _shadowMap);
        shader.setInt("dirLight.shadowMap", textureInd);
        shader.setMat4("dirLight.lightSpaceMatrix", generateProjectionMatrix());
        textureInd++;
    }
}

glm::mat4 DirectionalLight::generateProjectionMatrix() {
    // Generate projection matrix
    float near_plane = 0.1f, far_plane = 20.0f;
    glm::vec3 position = glm::vec3(0.0f, 8.0f, 5.0f);
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 
    glm::mat4 lightView = glm::lookAt(position, 
                                    position + direction,
                                    glm::vec3( 0.0f, 1.0f,  0.0f));  
    return lightProjection * lightView;
}