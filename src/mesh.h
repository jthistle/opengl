#ifndef __MESH__
#define __MESH__

#include <string>
#include <vector>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture.h"
#include "shader.h"

using std::string;
using std::vector;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

class Mesh {
    public:
        // mesh data
        vector<Vertex>       vertices;
        vector<unsigned int> indices;
        vector<Texture>      textures;
        float shininess { 0.0f };

        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
        void draw(Shader &shader);
        void setUseNormalMap(bool value) { _useNormalMap = value; }
    private:
        //  render data
        unsigned int VAO, VBO, EBO;
        bool _useNormalMap { true };

        void setupMesh();
};  

#endif /* __MESH__ */
