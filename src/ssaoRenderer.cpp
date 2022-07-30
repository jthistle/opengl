#include "ssaoRenderer.h"
#include <random>

float __lerp(float a, float b, float f) {
    return a + f * (b - a);
}  

void SSAORenderer::init(glm::ivec2 screenResolution) {
    if (_init) return;
    
    _screenRes = screenResolution;
    
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    for (unsigned int i = 0; i < NUM_SAMPLES; ++i) {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator)
        );
        sample  = glm::normalize(sample);
        sample *= randomFloats(generator);
        
        float scale = (float)i / (float)NUM_SAMPLES; 
        scale = __lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        _kernel.push_back(sample);  
    }

    // Generate noise
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            0.0f); 
        ssaoNoise.push_back(noise);
    }  

    glGenTextures(1, &_noiseTexture);
    glBindTexture(GL_TEXTURE_2D, _noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    // FBO
    glGenFramebuffers(1, &_FBO);  
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
  
    glGenTextures(1, &_colorBuffer);
    glBindTexture(GL_TEXTURE_2D, _colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenResolution.x, screenResolution.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ...and blur framebuffer
    glGenFramebuffers(1, &_blurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _blurFBO);
  
    glGenTextures(1, &_blurBuffer);
    glBindTexture(GL_TEXTURE_2D, _blurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenResolution.x, screenResolution.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Shader
    _renderShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/ssao.fs");
    _blurShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/ssaoBlur.fs");

    _init = true;
}

void SSAORenderer::draw(unsigned int gPosition, unsigned int gNormal, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
    glClear(GL_COLOR_BUFFER_BIT);    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _noiseTexture);

    _renderShader.use();
    
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        _renderShader.setVec3("samples[" + std::to_string(i) + "]", _kernel[i]);
    }
    _renderShader.setMat4("projection", projectionMatrix);
    _renderShader.setMat4("view", viewMatrix);
    _renderShader.setVec2("screenRes", _screenRes);
    
    _quad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Blur
    glBindFramebuffer(GL_FRAMEBUFFER, _blurFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _colorBuffer);
    _blurShader.use();
    _blurShader.setInt("ssaoInput", 0);

    _quad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}