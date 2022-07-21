#include <iostream>
#include <string>
#include <cmath>
#include <random>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "model.h"
#include "mesh.h"
#include "gameObject.h"

#include "renderer.h"

using glm::vec3;
using std::shared_ptr;


void processInput(GLFWwindow *window, Camera &camera, float dt)
{
    float cameraSpeed = 5.0f * dt;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.move(Camera::MoveDirection::FORWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.move(Camera::MoveDirection::BACKWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.move(Camera::MoveDirection::LEFT, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.move(Camera::MoveDirection::RIGHT, cameraSpeed);
}


int main()
{
    Renderer *renderer = Renderer::createRenderer(1200, 900);

    // Setup scene
    auto backpack = shared_ptr<GameObject>(new GameObject("../src/backpack/backpack.obj"));
    renderer->objects.push_back(backpack);

    // Plane
    Texture pixel("../src/pixel.png", GL_RGB);
    pixel.type = "texturesDiffuse";
    Texture pixelSpec("../src/pixel.png", GL_RGB);
    pixelSpec.type = "texutresSpecular";

    std::vector<Vertex> planeVerts = {
        { vec3(-1.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f) },
        { vec3(-1.0f, 0.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f) },
        { vec3( 1.0f, 0.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f) },
        { vec3( 1.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f) },
    };
    std::vector<unsigned int> planeIndices = { 0, 1, 2, 0, 2, 3 };
    std::vector<Texture> planeTextures = {
        pixel, pixelSpec
    };
    Mesh plane(planeVerts, planeIndices, planeTextures);
    plane.shininess = 128.0f;

    auto planeModel = Model(plane); 
    auto planeObj = shared_ptr<GameObject>(new GameObject(planeModel));
    renderer->objects.push_back(planeObj);

    planeObj->scale = glm::vec3(20.0f);
    planeObj->position = glm::vec3(0.0f, -1.7f, 0.0f);

    // Lights
    auto dirLight = shared_ptr<DirectionalLight>(new DirectionalLight(
        vec3(1.0f, 0.95f, 0.8f), 
        // vec3(0.0f),
        // vec3(1.0f, 0.4f, 0.2f), 
        0.05f,
        0.5f,
        1.0f,
        vec3(1.0f, -1.0f, -1.0f)
        // vec3(1.0f, -0.2f, -1.0f)
    ));
    renderer->dirLight = dirLight;

    auto light1 = shared_ptr<PointLight>(new PointLight(
        vec3(0.0f, 1.0f, 5.0f), vec3(1.0f, 0.0f, 0.0f), 0.1f, 0.5f, 1.0f, 25.0f
    ));
    // renderer->pointLights.push_back(light1);

    renderer->setSkyboxColor(vec3(0.02f, 0.1f, 0.3f));

    // Camera
    renderer->camera = Camera(50.0f, 1200.0f / 900.0f, 0.1f, 100.0f);

    // Timings
    float elapsedTime = 0.0f;
    float lastTimestamp = 0.0f;
    float deltaTime;

    // Render loop
    while(!renderer->shouldClose()) {
        // Update timings
        elapsedTime = (float) glfwGetTime();
        deltaTime = elapsedTime - lastTimestamp;
        lastTimestamp = elapsedTime;

        processInput(renderer->getWindow(), renderer->camera, deltaTime);

        renderer->draw();
    }
    
    Renderer::destroyRenderer();

    return 0;
}