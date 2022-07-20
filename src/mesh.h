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
    private:
        //  render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
};  

#endif /* __MESH__ */
