#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    loadShader(vertexPath, GL_VERTEX_SHADER);
    loadShader(fragmentPath, GL_FRAGMENT_SHADER);
    link();
}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
    loadShader(vertexPath, GL_VERTEX_SHADER);
    loadShader(geometryPath, GL_GEOMETRY_SHADER);
    loadShader(fragmentPath, GL_FRAGMENT_SHADER);
    link();
}

void Shader::loadShader(const char* path, GLuint shaderType)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string code;
    std::ifstream file;

    // ensure ifstream objects can throw exceptions:
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // Open file
        file.open(path);
        std::stringstream stream;
        // Read file to stream
        stream << file.rdbuf();		
        file.close();
        // Convert stream to string
        code = stream.str();		
    } catch(std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* codeRaw = code.c_str();

    // 2. compile shader
    unsigned int shader;
    int success;
    char infoLog[512];
    
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &codeRaw, NULL);
    glCompileShader(shader);

    // print compile errors if any
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR: Shader compilation failed for " << path << std::endl;
        std::cout << infoLog << std::endl;
    };

    _shaderParts.push_back(shader);
}

void Shader::link() {
    // Attach shaders
    ID = glCreateProgram();
    for (int i = 0; i < _shaderParts.size(); i++) {
        glAttachShader(ID, _shaderParts[i]);
    }
    glLinkProgram(ID);

    // Print linking errors if any
    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete linked shaders, no longer needed
    for (int i = 0; i < _shaderParts.size(); i++) {
        glDeleteShader(_shaderParts[i]);
    }
    _shaderParts.clear();
}

void Shader::use() const
{ 
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
} 

void Shader::setVec3(const std::string &name, float valX, float valY, float valZ) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), valX, valY, valZ); 
} 

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z); 
} 

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const
{ 
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); 
} 