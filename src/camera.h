#ifndef __CAMERA__
#define __CAMERA__

#include "global.h"
#include <glm/gtc/matrix_transform.hpp>
  
#include "shader.h"

class Camera
{
public:
    enum MoveDirection {
        FORWARD = 1,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
    };

    glm::vec3 position;
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

    void configureShader(Shader &shader);
};


#endif /* __CAMERA__ */
