#ifndef __BLOOMMANAGER__
#define __BLOOMMANAGER__

#include "global.h"

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

struct BloomMip {
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
};

class BloomManager {
public:
    BloomManager() {};
    ~BloomManager() {};
    bool init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength);
    void destroy();

    void bind();
    const std::vector<BloomMip>& mipChain() const { return _mipChain; }

private:
    bool _init { false };
    unsigned int _FBO;
    std::vector<BloomMip> _mipChain;
};

#endif /* __BLOOMMANAGER__ */
