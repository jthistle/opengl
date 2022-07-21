#ifndef __RENDERER__
#define __RENDERER__

#include <string>
#include <vector>
#include <memory> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "model.h"
#include "directionalLight.h"
#include "pointLight.h"

class Renderer {
private:
    GLFWwindow *_window;
    glm::vec2 _targetResolution;

    // Draw configuration
    glm::vec3 _skyboxColor;

    // Shaders
    Shader _objectShader;

public:
    Camera camera;
    std::vector<std::shared_ptr<Model>> models;
    std::shared_ptr<DirectionalLight> dirLight;
    std::vector<std::shared_ptr<PointLight>> pointLights;

private:
    void shaderConfigureLights();
    void shaderConfigureCameraViewpoint();
    void render();

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
