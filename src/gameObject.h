#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include "global.h"
#include <glm/gtc/matrix_transform.hpp>

#include "model.h"
#include "shader.h"

class Renderer;

class GameObject {
protected:
    Model _model;

public:
    glm::vec3 position;
    glm::vec3 scale { glm::vec3(1.0f) };
    glm::vec3 rotation;

    bool castsShadow { true };
    bool deferred { true };

public:
    GameObject() {};
    GameObject(Model &model);
    GameObject(string modelPath);

    virtual void draw(Shader &shader, const Renderer &renderer);

    // virtual void preRenderLoop(Shader &shader)
};

#endif /* __GAMEOBJECT__ */
