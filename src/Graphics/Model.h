#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "Shader.h"
#include <string>
#include <vector>

unsigned int TextureFromFile(const char *path, const std::string &directory);

class Model {
public:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;

    // Propiedades originales de estado y transformación
    bool isSelected = false;
    bool isLight = false;
    int pickingID;
    glm::vec3 pickingColor;

    std::string name;
    glm::vec3 position{ 0.0f };
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    
    glm::vec3 materialAmbient{ 1.0f };
    glm::vec3 materialDiffuse{ 1.0f };
    glm::vec3 materialSpecular{ 1.0f };
    float materialShininess = 32.0f;

    glm::vec3 lightDiffuse = glm::vec3(1.0f);
    glm::vec3 lightSpecular = glm::vec3(1.0f);
    glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
    float w = 1.0f;
    int lightingType = 0;

    glm::vec3 aabbMin{ FLT_MAX };
    glm::vec3 aabbMax{ -FLT_MAX };

    Model() = default; 
    Model(const std::string &path);
    
    void Draw(const Shader &shader) const;
    void CalculateAABB(); 

    void SetPickingID(int id) {
        pickingID = id;
        int r = (id & 0x000000FF) >>  0;
        int g = (id & 0x0000FF00) >>  8;
        int b = (id & 0x00FF0000) >> 16;
        pickingColor = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
    }

private:
    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};