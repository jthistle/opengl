#ifndef __SCREENQUAD__
#define __SCREENQUAD__

#include "global.h"

/**
 * @brief Simple container for screen quad geometry. Automatically generates
 * and configures vertex array, and can draw geometry.
 * 
 */
class ScreenQuad {
    bool _init { false };
    unsigned int _VBO, _VAO;

public:
    ScreenQuad() {};

    void init() {
        if (_init) return;

        const float vertexData[] = {
            // Screen pos Texture pos
            -1.0f, -1.0f, 0.0f, 0.0f, 
            -1.0f,  1.0f, 0.0f, 1.0f,
            1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);

        glBindVertexArray(_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

        // Attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        _init = true;
    };

    /**
     * @brief Draws the geometry of the screen quad. 
     * 
     */
    void draw() {
        if (!_init) init();

        glBindVertexArray(_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    };
};

#endif /* __SCREENQUAD__ */
