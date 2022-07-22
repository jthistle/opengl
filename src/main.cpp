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
using glm::vec2;
using std::shared_ptr;

static bool doNormalMap = false;

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

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        doNormalMap = !doNormalMap;

}


int main()
{
    Renderer *renderer = Renderer::createRenderer(1200, 900);

    // Setup scene
    // auto backpack = shared_ptr<GameObject>(new GameObject("../src/backpack/backpack.obj"));
    // renderer->objects.push_back(backpack);
    // backpack->scale = glm::vec3(1.0f);
    // backpack->position = glm::vec3(0.0f, 0.0f, 5.0f);

    // Plane
    Texture pixel("../src/brickwall.jpg", GL_RGB, true);
    pixel.type = "texturesDiffuse";
    Texture pixelSpec("../src/pixel.png", GL_RGB, false);
    pixelSpec.type = "texturesSpecular";
    Texture pixelNorm("../src/brickwall_normal.jpg", GL_RGB, false);
    pixelNorm.type = "textureNormal";

    std::vector<Vertex> planeVerts = {
        { vec3(-1.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f) },
        { vec3(-1.0f, 1.0f,  0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f) },
        { vec3( 1.0f, 1.0f,  0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f) },
        { vec3( 1.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f) },
    };
    std::vector<unsigned int> planeIndices = { 0, 1, 2, 0, 2, 3 };
    std::vector<Texture> planeTextures = {
        pixel, pixelSpec, pixelNorm
    };

    Mesh plane(planeVerts, planeIndices, planeTextures);
    plane.shininess = 128.0f;
    auto planeModel = Model(plane); 
    auto planeObj = shared_ptr<GameObject>(new GameObject(planeModel));
    renderer->objects.push_back(planeObj);

    planeObj->scale = glm::vec3(2.0f);
    // planeObj->position = glm::vec3(0.0f, -1.7f, 0.0f);

    // Lights
    auto dirLight = shared_ptr<DirectionalLight>(new DirectionalLight(
        // vec3(1.0f, 0.95f, 0.8f), 
        vec3(0.0f),
        // vec3(1.0f, 0.4f, 0.2f), 
        0.05f,
        0.5f,
        1.0f,
        vec3(1.0f, -1.0f, -1.0f)
        // vec3(1.0f, -0.2f, -1.0f)
    ));
    renderer->dirLight = dirLight;

    auto light1 = shared_ptr<PointLight>(new PointLight(
        vec3(0.0f, 1.0f, 5.0f), vec3(1.0f), 0.1f, 0.5f, 1.0f, 10.0f
    ));
    renderer->pointLights.push_back(light1);

    renderer->setSkyboxColor(vec3(0.02f, 0.1f, 0.3f));

    // Camera
    renderer->camera = Camera(50.0f, 1200.0f / 900.0f, 0.1f, 100.0f);

    // Timings
    float elapsedTime = 0.0f;
    float lastTimestamp = 0.0f;
    float deltaTime;

    // Render loop
    while(!renderer->shouldClose()) {
        planeObj->setUseNormalMap(doNormalMap);

        // Update timings
        elapsedTime = (float) glfwGetTime();
        deltaTime = elapsedTime - lastTimestamp;
        lastTimestamp = elapsedTime;

        renderer->dirLight->direction = glm::vec3(1.0f, -0.75f + 0.5f * sin(elapsedTime), -1.0f);
        light1->position = glm::vec3(1.0f * cos(elapsedTime), 1.0f * sin(elapsedTime), 1.0f);

        processInput(renderer->getWindow(), renderer->camera, deltaTime);

        renderer->draw();
    }
    
    Renderer::destroyRenderer();

    return 0;
}