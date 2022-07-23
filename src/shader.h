#ifndef __SHADER__
#define __SHADER__

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
  
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  

class Shader
{
private:
    std::vector<unsigned int> _shaderParts; 
    void init();
    void loadShader(const char* path, GLuint shaderType);
    void link();

public:
    // the program ID
    unsigned int ID;
  
    Shader() {}
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
    
    // use/activate the shader
    void use() const;
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float valX, float valY, float valZ) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;
};
  
#endif /* __SHADER__ */
