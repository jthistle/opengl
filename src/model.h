#ifndef __MODEL__
#define __MODEL__

#include "global.h"
#include <assimp/scene.h>
  
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
