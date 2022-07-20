#include <iostream>
#include <string>
#include <cmath>
#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "model.h"

static Camera camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.mouseCallback(window, xpos, ypos);
}

int main()
{
    // Init glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1200, 900, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
  
    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }    

    // gl config
    glViewport(0, 0, 1200, 900);
    glEnable(GL_DEPTH_TEST);  
    glEnable(GL_STENCIL_TEST);    

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);  

    // Debug information
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
    
    // Setup scene
    std::cout << "loading model" << std::endl;
    Model backpack("../src/backpack/backpack.obj");
    std::cout << "loaded model" << std::endl;

    // Lights
    DirectionalLight dirLight = DirectionalLight(
        glm::vec3(1.0f, 0.95f, 0.8f), 
        // glm::vec3(0.0f),
        // glm::vec3(1.0f, 0.4f, 0.2f), 
        0.1f,
        0.5f,
        1.0f,
        glm::vec3(1.0f, -1.0f, -1.0f)
        // glm::vec3(1.0f, -0.2f, -1.0f)
    );

    PointLight pointLights[] = {
        // PointLight(glm::vec3(2.0f, 1.0f, 5.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.1f, 0.5f, 1.0f, 50.0f),
        // PointLight(glm::vec3(-2.0f, -1.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 0.5f, 1.0f, 100.0f),
        // PointLight(glm::vec3(0.0f, 1.0f, 10.0f), glm::vec3(1.0f, 0.5f, 0.0f), 0.1f, 0.5f, 1.0f, 200.0f),
    };
    int numberPointLights = 0;

    glm::vec3 skyboxColor = glm::vec3(0.1f, 0.3f, 0.6f); 
    // glm::vec3 skyboxColor = glm::vec3(0.4f, 0.1f, 0.05f); 
    // glm::vec3 skyboxColor = glm::vec3(0.01f, 0.01f, 0.01f); 


    // Create element buffer object
    // unsigned int EBO;
    // glGenBuffers(1, &EBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // SHADERS
    Shader singleColorShader("../src/shaders/object.vs", "../src/shaders/singleColor.fs");
    Shader lightingShader("../src/shaders/object.vs", "../src/shaders/object.fs");
    lightingShader.use();
    lightingShader.setInt("numberPointLights", numberPointLights);
    dirLight.bind(lightingShader);
    for (int i = 0; i < numberPointLights; i++) {
        pointLights[i].bind(lightingShader, i);
    } 

    Shader lightCubeShader("../src/shaders/light.vs", "../src/shaders/light.fs");

    // Projections
    glm::mat4 model;

    // Camera
    camera = Camera(50.0f, 1200.0f / 900.0f, 0.1f, 100.0f);

    // Timings
    float elapsedTime = 0.0f;
    float lastTimestamp = 0.0f;
    float deltaTime;

    // Render loop
    glm::mat4 view;
    int loopCount = 0;
    while(!glfwWindowShouldClose(window))
    {
        loopCount += 1;
        elapsedTime = (float) glfwGetTime();
        deltaTime = elapsedTime - lastTimestamp;
        lastTimestamp = elapsedTime;

        processInput(window, camera, deltaTime);

        // Vertex transform matrices
        view = camera.generateView();

        // Clear colour buffer
        glClearColor(skyboxColor.x, skyboxColor.y, skyboxColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
        glStencilMask(0xFF); // enable writing to the stencil buffer
        lightingShader.use();

        // Update model
        // Update uniforms
        model = glm::mat4(1.0f);
        lightingShader.use();
        lightingShader.setMat4("model", model);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", camera.projection);
        lightingShader.setVec3("viewPos", camera.cameraPos);

        backpack.draw(lightingShader);

        // Draw scaled up
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00); 
        glDisable(GL_DEPTH_TEST);
        model = glm::scale(model, glm::vec3(1.02f));
        singleColorShader.use(); 
        singleColorShader.setMat4("model", model);
        singleColorShader.setMat4("view", view);
        singleColorShader.setMat4("projection", camera.projection);
        backpack.draw(singleColorShader);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);   
        glEnable(GL_DEPTH_TEST);  

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    

    // Cleanup
    glfwTerminate();

    return 0;
}