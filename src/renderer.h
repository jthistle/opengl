#ifndef __RENDERER__
#define __RENDERER__

#include <map>
#include <string>
#include <vector>
#include <memory> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "model.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "gameObject.h"

class Renderer {
private:
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    // Configuration (mutable)
    glm::vec3 _skyboxColor;
    bool _useNormalMaps { true };

    GLFWwindow *_window;
    glm::vec2 _targetResolution;

    // Shadow maps
    unsigned int _depthMapFBO;
    unsigned int _depthCubemap;

    // Deferred render buffers
    unsigned int _gBuffer, _gAlbedoSpec, _gNormal, _gPosition, _gDepth;
    
    // HDR processing buffers
    unsigned int _hdrBuffer, _hdrColorBuffer, _hdrDepthBuffer;
    
    // Postprocessing
    unsigned int _brightFBO, _brightBuffer;
    unsigned int _pingpongFBO[2], _pingpongBuffers[2];

    // Forward rendering mesh shader - legacy
    Shader _objectShader;
    // For rendering depth map for directional light
    Shader _depthShaderDir;
    // For rendering depth map for point light
    Shader _depthShaderPoint;
    // For rendering gBuffer (deferred render)
    Shader _gBufferShader;
    // For drawing and lighting gBuffer (deferred render)
    Shader _deferredShader;
    // For drawing HDR buffer to screen quad with tonemapping
    Shader _hdrShader;
    // Postprocessing
    Shader _gaussianShader;
    Shader _brightnessFilterShader;

    // temp debug    
    Shader _lightBoxShader;


    // Debug
    unsigned int _quadVAO, _quadVBO, _quadEBO, _quadTexture;
    Shader _quadShader;

public:
    Camera camera;
    std::vector<std::shared_ptr<GameObject>> objects;
    std::shared_ptr<DirectionalLight> dirLight;
    std::vector<std::shared_ptr<PointLight>> pointLights;

private:
    void shaderConfigureLights(Shader &shader);
    void shaderConfigureDeferred(Shader &shader);
    
    void renderAll(Shader &shader);
    void renderShadowCasters(Shader &shader);
    void renderDeferred(Shader &shader);
    void renderForward(Shader &shader);
    void renderGBuffer();

    void brightnessThreshold(unsigned int inTexture, unsigned int outFBO);

    void generateDepthMap(std::shared_ptr<DirectionalLight> light);
    void generateDepthMap(std::shared_ptr<PointLight> light);
    
    void drawDeferred();
    void drawForward();

    void renderQuad();

public:
    Renderer(int resX, int resY);
    ~Renderer();

    // API
    int init();
    void draw();

    bool shouldClose();
    
    // Callbacks
    void onFramebufferSizeChange(GLFWwindow* window, int width, int height);
    void onMouseMove(GLFWwindow* window, double xpos, double ypos);

    // Setters and getters
    void setSkyboxColor(glm::vec3 value) { _skyboxColor = value; }
    GLFWwindow* getWindow() { return _window; }
    void setUseNormalMaps(bool val) { _useNormalMaps = val; }

    // Debug
    void debugConfiguration();

    // Singleton management
    static Renderer* createRenderer(int resX, int resY);
    static void destroyRenderer();
};


#endif /* __RENDERER__ */
