#ifndef __SHADER__
#define __SHADER__

#include "global.h"

class Shader
{
private:
    vector<unsigned int> _shaderParts; 
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
    void setBool(const string &name, bool value) const;  
    void setInt(const string &name, int value) const;   
    void setFloat(const string &name, float value) const;
    void setVec2(const string &name, const glm::vec2 &value) const;
    void setVec2(const string &name, float valX, float valY) const;
    void setVec3(const string &name, const glm::vec3 &value) const;
    void setVec3(const string &name, float valX, float valY, float valZ) const;
    void setMat4(const string &name, const glm::mat4 &value) const;

    bool isValid() const;
};
  
#endif /* __SHADER__ */
