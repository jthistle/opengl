#ifndef __MODEL__
#define __MODEL__

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
  
#include <string>
#include <vector>

#include "mesh.h"

  
class Model 
{
    public:
        Model() { }
        Model(Mesh &mesh) { meshes.push_back(mesh); }
        Model(std::string path) {
            loadModel(path);
        }
        void draw(Shader &shader);	
        void setUseNormalMap(bool val);
        
    private:
        // model data
        vector<Texture> textures_loaded; 
        std::vector<Mesh> meshes;
        std::string directory;

        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, bool gammaCorrect);
};

#endif /* __MODEL__ */
