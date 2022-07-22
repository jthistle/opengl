#include "gameObject.h"

using std::string;

GameObject::GameObject(Model &model) {
    _model = model;
}

GameObject::GameObject(string modelPath) {
    _model = Model(modelPath);
}

void GameObject::draw(Shader &shader) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); 
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); 
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); 
    shader.setMat4("model", modelMatrix);
    _model.draw(shader);
}