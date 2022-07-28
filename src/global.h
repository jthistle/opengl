#ifndef __GLOBAL__
#define __GLOBAL__

/**
 * @file global.h
 * @brief Includes which will be needed in pretty much every file.
 * 
 */

// stdlib
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

// aliases
using std::string;
using std::vector;
using std::shared_ptr;

// GL & graphics
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#endif /* __GLOBAL__ */
