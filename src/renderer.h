#ifndef __RENDERER__
#define __RENDERER__

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

    GLFWwindow *_window;
    glm::vec2 _targetResolution;

    // Shadows
    unsigned int _depthMap;
    unsigned int _depthMapFBO;
    unsigned int _depthCubemap;

    // Draw configuration
    glm::vec3 _skyboxColor;

    // Shaders
    Shader _objectShader;
    Shader _depthShaderDir;
    Shader _depthShaderPoint;
    Shader _quadShader;

    // Debug
    unsigned int _quadVAO, _quadVBO, _quadEBO, _quadTexture;

public:
    Camera camera;
    std::vector<std::shared_ptr<GameObject>> objects;
    std::shared_ptr<DirectionalLight> dirLight;
    std::vector<std::shared_ptr<PointLight>> pointLights;

private:
    void shaderConfigureLights();
    void shaderConfigureCameraViewpoint();
    void render(Shader &shader);

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

    // Debug
    void debugConfiguration();

    // Singleton
    static Renderer* createRenderer(int resX, int resY);
    static void destroyRenderer();
};


#endif /* __RENDERER__ */
