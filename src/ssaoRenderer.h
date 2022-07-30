#ifndef __SSAORENDERER__
#define __SSAORENDERER__

#include "global.h"
#include "shader.h"
#include "screenQuad.h"

class SSAORenderer {
    const unsigned int NUM_SAMPLES = 64;

    bool _init { false };
    glm::ivec2 _screenRes;
    unsigned int _FBO, _colorBuffer, _noiseTexture;
    unsigned int _blurFBO, _blurBuffer;
    vector<glm::vec3> _kernel;
    Shader _renderShader;
    Shader _blurShader;
    ScreenQuad _quad; 

public:
    SSAORenderer() {};
    void init(glm::ivec2 screenResolution);
    void draw(unsigned int gPosition, unsigned int gNormal, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);

    unsigned int getTexture() const { return _blurBuffer; }

};

#endif /* __SSAORENDERER__ */
