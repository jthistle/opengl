#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.h"
#include "shader.h"


class GameObject {
    Model _model;

public:
    glm::vec3 position;
    glm::vec3 scale { glm::vec3(1.0f) };
    glm::vec3 rotation;

public:
    GameObject(Model &model);
    GameObject(std::string modelPath);

    void draw(Shader &shader);
};

#endif /* __GAMEOBJECT__ */
