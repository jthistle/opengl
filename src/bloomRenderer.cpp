#include "bloomRenderer.h"

bool BloomRenderer::init(unsigned int windowWidth, unsigned int windowHeight) {
    if (_init) return true;

    _srcViewportSize = glm::ivec2(windowWidth, windowHeight);
    _srcViewportSizeFloat = glm::vec2((float)windowWidth, (float)windowHeight);

    // Framebuffer
    const unsigned int num_bloom_mips = 5; // Experiment with this value
    bool status = _manager.init(windowWidth, windowHeight, num_bloom_mips);
    if (!status) {
        std::cerr << "Failed to initialize bloom FBO - cannot create bloom renderer!\n";
        return false;
    }

    // Shaders
    _downsampleShader = Shader("../src/shaders/scaleCommon.vs", "../src/shaders/downsample.fs");
    _upsampleShader = Shader("../src/shaders/scaleCommon.vs", "../src/shaders/upsample.fs");

    // Downsample
    _downsampleShader.use();
    _downsampleShader.setInt("srcTexture", 0);

    // Upsample
    _upsampleShader.use();
    _upsampleShader.setInt("srcTexture", 0);

    // Quad setup
    float quadVertices[] = {
        // upper-left triangle
        -1.0f, -1.0f, 0.0f, 0.0f, // position, texcoord
        -1.0f,  1.0f, 0.0f, 1.0f,
        1.0f,  1.0f, 1.0f, 1.0f,
        // lower-right triangle
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &_quadVAO);
    glGenBuffers(1, &_quadVBO);

    glBindVertexArray(_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    std::cout << "bloom renderer: init with width " << windowWidth << " and height " << windowHeight << std::endl;

    _init = true;
    return true;
}

void BloomRenderer::destroy()
{
    _manager.destroy();
    _init = false;
}

void BloomRenderer::renderBloomTexture(unsigned int srcTexture, float filterRadius)
{
    _manager.bind();

    renderDownsamples(srcTexture);
    renderUpsamples(filterRadius);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Restore viewport
    glViewport(0, 0, _srcViewportSize.x, _srcViewportSize.y);
}

GLuint BloomRenderer::bloomTexture()
{
    return _manager.mipChain()[0].texture;
}

void BloomRenderer::renderDownsamples(unsigned int srcTexture)
{
    auto mipChain = _manager.mipChain();

    _downsampleShader.use();
    _downsampleShader.setVec2("srcResolution", _srcViewportSize);

    // Bind srcTexture (HDR color buffer) as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    // Progressively downsample through the mip chain
    for (int i = 0; i < mipChain.size(); i++)
    {
        const BloomMip& mip = mipChain[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        glBindVertexArray(_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Set current mip resolution as srcResolution for next iteration
        _downsampleShader.setVec2("srcResolution", mip.size);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture);
    }
}

void BloomRenderer::renderUpsamples(float filterRadius)
{
    auto mipChain = _manager.mipChain();

    _upsampleShader.use();
    _upsampleShader.setFloat("filterRadius", filterRadius);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = mipChain.size() - 1; i > 0; i--)
    {
        const BloomMip& mip = mipChain[i];
        const BloomMip& nextMip = mipChain[i-1];

        // Bind viewport and texture from where to read
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, nextMip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        glBindVertexArray(_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // Disable additive blending
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
    glDisable(GL_BLEND);
}