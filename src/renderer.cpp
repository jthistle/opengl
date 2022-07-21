#include "renderer.h"

#include <iostream>
#include <cmath>

#include <stb_image.h>

#include "shader.h"
#include "texture.h"

using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;
using std::shared_ptr;


// Singleton management
static Renderer *renderer = NULL;

Renderer* Renderer::createRenderer(int resX, int resY) {
    if (renderer != NULL) {
        std::cout << "Illegal attempt to create renderer - renderer already created!" << std::endl;
        return NULL;
    }

    renderer = new Renderer(resX, resY);
    if (renderer->init() != 0) {
        destroyRenderer();
    }

    return renderer;
}

void Renderer::destroyRenderer() {
    delete renderer;
    renderer = NULL;
}


// Callback functions
void __onFramebufferSizeChange(GLFWwindow* window, int width, int height) {
    renderer->onFramebufferSizeChange(window, width, height);
}  

void __onMouseMove(GLFWwindow* window, double xpos, double ypos) {
    renderer->onMouseMove(window, xpos, ypos);
}  


//
// class Renderer
//
Renderer::Renderer(int resX, int resY) {
    _targetResolution = vec2(resX, resY);
}

Renderer::~Renderer() {
    glfwTerminate();
}

void Renderer::onFramebufferSizeChange(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}  

void Renderer::onMouseMove(GLFWwindow* window, double xpos, double ypos) {
    camera.mouseCallback(window, xpos, ypos);
}  

int Renderer::init() {
    // GLFW configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create window
    _window = glfwCreateWindow(_targetResolution.x, _targetResolution.y, "LearnOpenGL", NULL, NULL);
    if (_window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(_window);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }    

    // gl config
    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
    glEnable(GL_DEPTH_TEST);  
    glEnable(GL_STENCIL_TEST);    
    glEnable(GL_FRAMEBUFFER_SRGB);  // gamma correction 
    
    // Capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Register callbacks
    glfwSetFramebufferSizeCallback(_window, __onFramebufferSizeChange);
    glfwSetCursorPosCallback(_window, __onMouseMove);

    // Debug config
    debugConfiguration();

    // Compile basic shader
    _objectShader = Shader("../src/shaders/object.vs", "../src/shaders/object.fs");

    // Set default dirLight
    dirLight = shared_ptr<DirectionalLight>(new DirectionalLight(
        vec3(0.0f), 0.1f, 0.5f, 1.0f, vec3(0.0f)
    ));

    return 0;
}

void Renderer::shaderConfigureLights() {
    int numberPointLights = pointLights.size();

    _objectShader.use();
    _objectShader.setInt("numberPointLights", numberPointLights);
    dirLight->bind(_objectShader);
    for (int i = 0; i < numberPointLights; i++) {
        pointLights[i]->bind(_objectShader, i);
    } 
}

void Renderer::shaderConfigureCameraViewpoint() {
    _objectShader.use();
    _objectShader.setMat4("view", camera.generateView());
    _objectShader.setMat4("projection", camera.projection); 
    _objectShader.setVec3("viewPos", camera.cameraPos);
}

/**
 * @brief Performs a render pass. 
 * 
 * This is not an external function! Shaders must be configured accordingly before calling.
 */
void Renderer::render() {
    _objectShader.use();
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        (*i)->draw(_objectShader);
    }
}

void Renderer::draw() {
    glm::mat4 model;
    glm::mat4 view;

    // Clear buffers
    glClearColor(_skyboxColor.x, _skyboxColor.y, _skyboxColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    shaderConfigureLights();
    shaderConfigureCameraViewpoint();
    render();

    glfwSwapBuffers(_window);
    glfwPollEvents();    
}

bool Renderer::shouldClose() {
    return glfwWindowShouldClose(_window);
}

void Renderer::debugConfiguration() {
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
}