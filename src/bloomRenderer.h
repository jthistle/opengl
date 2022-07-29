#ifndef __BLOOMRENDERER__
#define __BLOOMRENDERER__

#include "global.h"

#include "shader.h"
#include "bloomManager.h"

class BloomRenderer {
public:
    BloomRenderer() {};
    ~BloomRenderer() {};

    bool init(unsigned int windowWidth, unsigned int windowHeight);
    void destroy();

    void renderBloomTexture(unsigned int srcTexture, float filterRadius);
    unsigned int bloomTexture();

private:
    void renderDownsamples(unsigned int srcTexture);
    void renderUpsamples(float filterRadius);

    bool _init { false };
    BloomManager _manager;
    glm::ivec2 _srcViewportSize;
    glm::vec2 _srcViewportSizeFloat;
    Shader _downsampleShader;
    Shader _upsampleShader;
    
    unsigned int _quadVBO, _quadVAO;
};

#endif /* __BLOOMRENDERER__ */
