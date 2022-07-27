#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    GameObject(std::string modelPath);

    virtual void draw(Shader &shader, const Renderer &renderer);

    // virtual void preRenderLoop(Shader &shader)
};

#endif /* __GAMEOBJECT__ */
