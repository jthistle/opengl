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

//
// Singleton management
//
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
static void __onFramebufferSizeChange(GLFWwindow* window, int width, int height) {
    renderer->onFramebufferSizeChange(window, width, height);
}  

static void __onMouseMove(GLFWwindow* window, double xpos, double ypos) {
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

    // Init gBuffer
    glGenFramebuffers(1, &_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);
    
    // - position color buffer
    glGenTextures(1, &_gPosition);
    glBindTexture(GL_TEXTURE_2D, _gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gPosition, 0);
    
    // - normal color buffer
    glGenTextures(1, &_gNormal);
    glBindTexture(GL_TEXTURE_2D, _gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gNormal, 0);
    
    // - color + specular color buffer
    glGenTextures(1, &_gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, _gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _gAlbedoSpec, 0);

    // Attach depth map to framebuffer
    glGenTextures(1, &_gDepth);
    glBindTexture(GL_TEXTURE_2D, _gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _targetResolution.x, _targetResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _gDepth, 0);
    
    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // Debug config
    debugConfiguration();

    // Compile basic shaders
    _objectShader = Shader("../src/shaders/object.vs", "../src/shaders/object.fs");
    _depthShaderDir = Shader("../src/shaders/depthShaderDirectional.vs", "../src/shaders/depthShaderDirectional.fs");
    _depthShaderPoint = Shader("../src/shaders/depthShaderPoint.vs", "../src/shaders/depthShaderPoint.fs", "../src/shaders/depthShaderPoint.gs");
    _quadShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/simpleQuad.fs");
    _gBufferShader = Shader("../src/shaders/gBuffer.vs", "../src/shaders/gBuffer.fs");
    _deferredShader = Shader("../src/shaders/objectDef.vs", "../src/shaders/objectDef.fs");

    // Set default dirLight
    dirLight = shared_ptr<DirectionalLight>(new DirectionalLight(
        vec3(0.0f), 0.1f, 0.5f, 1.0f, vec3(0.0f), true
    ));

    // Quad stuff (for debug)
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

    // Debug: choose texture to display on quad
    _quadTexture = _gNormal;

    return 0;
}

/**
 * @brief Sets uniforms for shaders requiring information about lighting.
 * 
 */
void Renderer::shaderConfigureLights(Shader &shader) {
    int numberPointLights = pointLights.size();
    int textureNumber = 8;

    shader.use();
    dirLight->bind(shader, textureNumber);
    shader.setInt("numberPointLights", numberPointLights);
    for (int i = 0; i < numberPointLights; i++) {
        pointLights[i]->bind(shader, i, textureNumber);
    } 
}

/**
 * @brief Sets uniforms for shaders requiring information about the camera.
 * 
 */
void Renderer::shaderConfigureCameraViewpoint(Shader &shader) {
    shader.use();
    shader.setMat4("view", camera.generateView());
    shader.setMat4("projection", camera.projection); 
    shader.setVec3("viewPos", camera.cameraPos);
}

/**
 * @brief Sets uniforms for shaders requiring access to the gBuffer.
 * 
 */
void Renderer::shaderConfigureDeferred(Shader &shader) {
    shader.use();
    
    // Load gBuffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gAlbedoSpec);
    shader.setInt("gAlbedoSpec", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gNormal);
    shader.setInt("gNormal", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gPosition);
    shader.setInt("gPosition", 2);

    shader.setVec3("viewPos", camera.cameraPos);
    shader.setVec3("skyboxColor", _skyboxColor);
}

/**
 * @brief Renders all objects in the scene. 
 * 
 * This is not an external function! Shaders must be configured accordingly before calling.
 */
void Renderer::render(Shader &shader) {
    shader.use();
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        (*i)->draw(shader);
    }
}

/**
 * @brief Draws the texture referenced by `_quadTexture` to the screen quad. 
 * 
 */
void Renderer::renderQuad() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _quadTexture);

    _quadShader.use();
    _quadShader.setInt("quadTexture", 0);

    glBindVertexArray(_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/**
 * @brief Renders the scene to the gBuffer. 
 * 
 */
void Renderer::renderGBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);  
    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderConfigureCameraViewpoint(_gBufferShader);
    _gBufferShader.setBool("useNormalMaps", _useNormalMaps);
    render(_gBufferShader);
}

/**
 * @brief Generates the depth map for a directional light. 
 * 
 * @param light 
 * @param framebuf 
 * @param texture 
 */
void Renderer::generateDepthMap(shared_ptr<DirectionalLight> light) {
    light->configureForDepthMap(_depthShaderDir, _depthMapFBO);
    render(_depthShaderDir);
}

/**
 * @brief Generates the depth map for a point light. 
 * 
 * @param light 
 * @param framebuf 
 * @param texture 
 */
void Renderer::generateDepthMap(shared_ptr<PointLight> light) {
    if (light->getCastsShadow()) {
        light->configureForDepthMap(_depthShaderPoint, _depthMapFBO);
        render(_depthShaderPoint);
    }
}

/**
 * @brief Draws the scene stored in the gBuffer to the screen quad.
 * 
 */
void Renderer::drawDeferred() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
    // Clear depth buffer - colour buffer will be overwritten
    glClear(GL_DEPTH_BUFFER_BIT);
    // Configure shaders
    shaderConfigureDeferred(_deferredShader);
    shaderConfigureLights(_deferredShader);
    
    // Draw onto quad
    glBindVertexArray(_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/**
 * @brief Render and draw the scene to the screen. 
 * 
 * This is the entry point for rendering. It should be called once per render loop.
 */
void Renderer::draw() {
    // Directional light depth map 
    generateDepthMap(dirLight);

    // Point light depth map (just first one for now)
    for (int i = 0; i < pointLights.size(); i++) {
        generateDepthMap(pointLights[i]);
    }

    // gBuffer
    renderGBuffer();

    // Visible render pass
    drawDeferred();

#if 0
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    renderQuad();
#endif

    glfwSwapBuffers(_window);
    glfwPollEvents();    
}

/**
 * @brief Whether the window has recieved a close event and should close. 
 */
bool Renderer::shouldClose() {
    return glfwWindowShouldClose(_window);
}

/**
 * @brief Output some information about GL's limits on this machine. 
 * 
 */
void Renderer::debugConfiguration() {
    int integer;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &integer);
    std::cout << "Maximum nr of vertex attributes supported: " << integer << std::endl;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &integer);
    std::cout << "Maximum nr of textures in frag shader: " << integer << std::endl;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &integer);
    std::cout << "Maximum nr of color attachments: " << integer << std::endl;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &integer);
    std::cout << "Maximum nr of frag uniform components: " << integer << std::endl;
}