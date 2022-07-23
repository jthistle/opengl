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

    // Shadows setup
    glGenFramebuffers(1, &_depthMapFBO);  
    glGenTextures(1, &_depthMap);
    glBindTexture(GL_TEXTURE_2D, _depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // Point light shadows cubemap
    glGenTextures(1, &_depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);  
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // Debug config
    debugConfiguration();

    // Compile basic shaders
    _objectShader = Shader("../src/shaders/object.vs", "../src/shaders/object.fs");
    _depthShaderDir = Shader("../src/shaders/depthShaderDirectional.vs", "../src/shaders/depthShaderDirectional.fs");
    _depthShaderPoint = Shader("../src/shaders/depthShaderPoint.vs", "../src/shaders/depthShaderPoint.fs", "../src/shaders/depthShaderPoint.gs");
    _quadShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/simpleQuad.fs");

    // Set default dirLight
    dirLight = shared_ptr<DirectionalLight>(new DirectionalLight(
        vec3(0.0f), 0.1f, 0.5f, 1.0f, vec3(0.0f)
    ));

    // Quad stuff
    float quadVerts[] = {
        // Screen pos       Texture coord
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    };

    unsigned int quadInds[] = {
        0, 1, 2,
        0, 2, 3,
    };

    glGenVertexArrays(1, &_quadVAO);
    glGenBuffers(1, &_quadVBO);
    glGenBuffers(1, &_quadEBO);
  
    glBindVertexArray(_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);

    glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(float), &quadVerts[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &quadInds[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    _quadTexture = _depthMap;

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
    _objectShader.setMat4("lightSpaceMatrix", dirLight->generateProjectionMatrix());

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, _depthMap);
    _objectShader.setInt("shadowMap", 15);

    // Temp point light setup
    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _depthCubemap);
    _objectShader.setInt("shadowMapPoint", 14); // debug
    _objectShader.setFloat("far_plane", pointLights.at(0)->getRange()); // debug
}

/**
 * @brief Performs a render pass. 
 * 
 * This is not an external function! Shaders must be configured accordingly before calling.
 */
void Renderer::render(Shader &shader) {
    shader.use();
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        (*i)->draw(shader);
    }
}

void Renderer::renderQuad() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _quadTexture);

    _quadShader.use();
    _quadShader.setInt("quadTexture", 0);

    glBindVertexArray(_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::draw() {
    // Generate depth map (just from directional light for now)
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    _depthShaderDir.use();
    _depthShaderDir.setMat4("lightSpaceMatrix", dirLight->generateProjectionMatrix());
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    render(_depthShaderDir);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Point light depth map
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    _depthShaderPoint.use();
    _depthShaderPoint.setVec3("lightPos", pointLights.at(0)->position);
    _depthShaderPoint.setFloat("far_plane", pointLights.at(0)->getRange());
    auto matrices = pointLights.at(0)->generateProjectionMatrices();
    for (int i = 0; i < matrices.size(); i++) {
        _depthShaderPoint.setMat4("shadowMatrices[" + std::to_string(i) + "]", matrices[i]);
    }
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    render(_depthShaderPoint);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Visible render pass
    glClearColor(_skyboxColor.x, _skyboxColor.y, _skyboxColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
    shaderConfigureLights();
    shaderConfigureCameraViewpoint();
    render(_objectShader);

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // renderQuad();

    glfwSwapBuffers(_window);
    glfwPollEvents();    
}

bool Renderer::shouldClose() {
    return glfwWindowShouldClose(_window);
}

void Renderer::debugConfiguration() {
    int integer;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &integer);
    std::cout << "Maximum nr of vertex attributes supported: " << integer << std::endl;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &integer);
    std::cout << "Maximum nr of textures in frag shader: " << integer << std::endl;
}