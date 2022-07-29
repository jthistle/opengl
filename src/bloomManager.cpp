#include "bloomManager.h"

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

bool BloomManager::init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength) {
    if (_init) return true;

    glGenFramebuffers(1, &_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

    glm::vec2 mipSize((float)windowWidth, (float)windowHeight);
    glm::ivec2 mipIntSize((int)windowWidth, (int)windowHeight);

    // Safety check
    if (windowWidth > (unsigned int)INT_MAX || windowHeight > (unsigned int)INT_MAX) {
        std::cerr << "Window size conversion overflow - cannot build bloom FBO!\n";
        return false;
    }

    for (unsigned int i = 0; i < mipChainLength; i++)
    {
        BloomMip mip;

        mipSize *= 0.5f;
        mipIntSize /= 2;
        mip.size = mipSize;
        mip.intSize = mipIntSize;

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // we are downscaling an HDR color buffer, so we need a float texture format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, mipIntSize.x, mipIntSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        _mipChain.emplace_back(mip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, _mipChain[0].texture, 0);

    // setup attachments
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    // check completion status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("gbuffer FBO error, status: 0x\%x\n", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _init = true;

    return true;
}

void BloomManager::destroy()
{
    for (int i = 0; i < _mipChain.size(); i++) {
        glDeleteTextures(1, &_mipChain[i].texture);
        _mipChain[i].texture = 0;
    }
    glDeleteFramebuffers(1, &_FBO);
    _FBO = 0;
    _init = false;
}

void BloomManager::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
}
