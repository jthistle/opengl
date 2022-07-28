#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "texture.h"

using glm::vec2;
using glm::vec3;

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
        return 1;
    }
    glfwMakeContextCurrent(_window);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return 1;
    }    

    // gl config
    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
    glEnable(GL_DEPTH_TEST);  
    // glEnable(GL_STENCIL_TEST);    
    // glEnable(GL_FRAMEBUFFER_SRGB);  // gamma correction 
    
    // Capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Register callbacks
    glfwSetFramebufferSizeCallback(_window, __onFramebufferSizeChange);
    glfwSetCursorPosCallback(_window, __onMouseMove);


    //
    // Frame buffer for shadow mapping
    //
    glGenFramebuffers(1, &_depthMapFBO);  

    //
    // Init gBuffer
    //
    glGenFramebuffers(1, &_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);
    
    // Position buffer
    glGenTextures(1, &_gPosition);
    glBindTexture(GL_TEXTURE_2D, _gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gPosition, 0);
    
    // Normal buffer
    glGenTextures(1, &_gNormal);
    glBindTexture(GL_TEXTURE_2D, _gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gNormal, 0);
    
    // Colour and specular buffer
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
    
    // Attach the colour buffers 
    unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    //
    // HDR buffer setup
    //
    glGenFramebuffers(1, &_hdrBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _hdrBuffer);
    
    // Normal colour buffer - we use format GL_RGBA15F so that we have range greater than [0, 1]
    glGenTextures(1, &_hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, _hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _hdrColorBuffer, 0);

    // Attach depth map to framebuffer
    glGenTextures(1, &_hdrDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, _hdrDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _targetResolution.x, _targetResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _hdrDepthBuffer, 0);

    unsigned int attachments2[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments2);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    //
    // Setup bright colour buffer
    //
    glGenFramebuffers(1, &_brightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _brightFBO);
    glGenTextures(1, &_brightBuffer);
    glBindTexture(GL_TEXTURE_2D, _brightBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _brightBuffer, 0);

    unsigned int attachments3[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    //
    // Init pingpong buffer
    //
    glGenFramebuffers(2, _pingpongFBO);
    glGenTextures(2, _pingpongBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, _pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, _pingpongBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, _targetResolution.x, _targetResolution.y, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pingpongBuffers[i], 0
        );
    }

    // Debug config
    debugConfiguration();

    // Compile basic shaders
    _objectShader = Shader("../src/shaders/object.vs", "../src/shaders/object.fs");
    _depthShaderDir = Shader("../src/shaders/depthShaderDirectional.vs", "../src/shaders/depthShaderDirectional.fs");
    _depthShaderPoint = Shader("../src/shaders/depthShaderPoint.vs", "../src/shaders/depthShaderPoint.fs", "../src/shaders/depthShaderPoint.gs");
    _quadShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/simpleQuad.fs");
    _gBufferShader = Shader("../src/shaders/gBuffer.vs", "../src/shaders/gBuffer.fs");
    _deferredShader = Shader("../src/shaders/objectDef.vs", "../src/shaders/objectDef.fs");
    _hdrShader = Shader("../src/shaders/hdr.vs", "../src/shaders/hdr.fs");
    _gaussianShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/gaussian.fs");
    _brightnessFilterShader = Shader("../src/shaders/simpleQuad.vs", "../src/shaders/brightFilter.fs");

    _lightBoxShader = Shader("../src/shaders/lightBox.vs", "../src/shaders/lightBox.fs");

    // Set default dirLight
    dirLight = shared_ptr<DirectionalLight>(new DirectionalLight(
        vec3(0.0f), 0.1f, 0.5f, 1.0f, vec3(0.0f), true
    ));

    //
    // Screen quad setup
    //
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

    shader.setVec3("viewPos", camera.position);
    shader.setVec3("skyboxColor", _skyboxColor);
}

/**
 * @brief Renders all objects in the scene. 
 * 
 * This is not an external function! Shaders must be configured accordingly before calling.
 */
void Renderer::renderAll(Shader &shader) {
    shader.use();
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        (*i)->draw(shader, *this);
    }
}

void Renderer::renderShadowCasters(Shader &shader) {
    shader.use();
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        if (!(*i)->castsShadow) continue;
        (*i)->draw(shader, *this);
    }
}

void Renderer::renderDeferred(Shader &shader) {
    shader.use();
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        if (!(*i)->deferred) continue;
        (*i)->draw(shader, *this);
    }
}

void Renderer::renderForward(Shader &shader) {
    shader.use();
    int x = 0;
    for (auto i = objects.begin(); i != objects.end(); ++i) {
        if ((*i)->deferred) continue;

        // HACK temporary debug
        if (x++ == 0)
            _lightBoxShader.setVec3("lightColor", 20.0f, 0.0f, 0.0f); 
        else
            _lightBoxShader.setVec3("lightColor", 0.0f, 5.0f, 0.0f);

        (*i)->draw(shader, *this);
    }
}

/**
 * @brief Draws the texture referenced by `_quadTexture` to the screen quad. 
 * 
 * This is a debug tool, intended to be used to draw arbitrary textures to the screen.
 */
void Renderer::renderQuad() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _quadTexture);

    _quadShader.use();
    _quadShader.setInt("quadTexture", 0);

    glBindVertexArray(_quadVAO);
    glBindTexture(GL_TEXTURE_2D, _quadTexture);
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

    _gBufferShader.use();
    _gBufferShader.setBool("useNormalMaps", _useNormalMaps);
    camera.configureShader(_gBufferShader);
    renderDeferred(_gBufferShader);
}

/**
 * @brief Generates the depth map for a directional light. 
 * 
 * @param light 
 * @param framebuf 
 * @param texture 
 */
void Renderer::generateDepthMap(shared_ptr<DirectionalLight> light) {
    if (light->getCastsShadow()) {
        light->configureForDepthMap(_depthShaderDir, _depthMapFBO);
        renderShadowCasters(_depthShaderDir);
    }

    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
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
        renderShadowCasters(_depthShaderPoint);
    }

    glViewport(0, 0, _targetResolution.x, _targetResolution.y);
}

/**
 * @brief Draws the scene stored in the gBuffer to the HDR buffer.
 * 
 */
void Renderer::drawDeferred() {
    glBindFramebuffer(GL_FRAMEBUFFER, _hdrBuffer);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear depth buffer - colour buffer will be overwritten
    
    // Configure shaders
    shaderConfigureDeferred(_deferredShader);
    shaderConfigureLights(_deferredShader);
    
    // Draw onto quad
    glBindVertexArray(_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Forward render pass - draws objects marked for forward rendering. 
 * 
 * This occurs after the deferred pass.
 */
void Renderer::drawForward() {
    // Copy gBuffer depth map
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _hdrBuffer); // write to default framebuffer
    glBlitFramebuffer(0, 0, _targetResolution.x, _targetResolution.y, 0, 0, _targetResolution.x, _targetResolution.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, _hdrBuffer);

    // temp debug
    camera.configureShader(_lightBoxShader);
    _lightBoxShader.setVec3("lightColor", glm::vec3(1.0f, 0.0f, 0.0f));

    renderForward(_lightBoxShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Applies a threshold to the _hdrColorBuffer and writes to _brightBuffer. 
 * 
 * @param inTexture GL_TEXTURE_2D which is to be thresholded. 
 * @param outFBO FBO with an attached color buffer in location 0 of the same size as inTexture.
 */
void Renderer::brightnessThreshold(unsigned int inTexture, unsigned int outFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, outFBO);

    _brightnessFilterShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inTexture);
    _brightnessFilterShader.setInt("colorBuffer", 0);
    
    // Draw
    glBindVertexArray(_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Unbinds
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Render and draw the scene to the screen. 
 * 
 * This is the entry point for rendering. It should be called once per render loop.
 */
void Renderer::draw() {
    // Directional light depth map 
    generateDepthMap(dirLight);

    // Point light depth maps 
    for (int i = 0; i < pointLights.size(); i++) {
        generateDepthMap(pointLights[i]);
    }

    // gBuffer
    renderGBuffer();

    // Visible render pass
    drawDeferred();

    // Forward pass
    drawForward();

    // Brightness pass
    brightnessThreshold(_hdrColorBuffer, _brightFBO);

    // Blur bright texture
    bool horizontal = true, first_iteration = true;
    int amount = 20;
    _gaussianShader.use();
    glActiveTexture(GL_TEXTURE0);
    _gaussianShader.setInt("image", 0);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _pingpongFBO[horizontal]); 
        _gaussianShader.setInt("horizontal", horizontal);
        glBindTexture(
            GL_TEXTURE_2D, first_iteration ? _brightBuffer : _pingpongBuffers[!horizontal]
        ); 
        
        // draw to quad
        glBindVertexArray(_quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw HDR buffer onto quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    _hdrShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _hdrColorBuffer);
    _hdrShader.setInt("colorBuffer", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _pingpongBuffers[0]);
    _hdrShader.setInt("bloomBlur", 1);
    glBindVertexArray(_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

#if 0
    _quadTexture = _pingpongBuffers[0];
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