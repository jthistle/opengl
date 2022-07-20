#ifndef __CAMERA__
#define __CAMERA__

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
  
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  

class Camera
{
public:
    enum MoveDirection {
        FORWARD = 1,
        BACKWARD,
        LEFT,
        RIGHT,
    };

    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    
    bool firstMouse { true };
    glm::vec2 lastMouse; 
    float yaw {0.0f};
    float pitch {0.0f};

    glm::mat4 projection;
  
    Camera() {};
    Camera(float fov, float aspect, float near, float far);
    glm::mat4 generateView();
    void move(MoveDirection direction, float distance);
    void mouseCallback(GLFWwindow* window, double xpos, double ypos);
};


#endif /* __CAMERA__ */
