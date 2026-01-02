#pragma once
#include "../Graphics/Shader.h"
#include "../Core/Camera.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Model {
    GLuint VAO = 0, VBO = 0, EBO = 0;
    bool isSelected = false;
    bool isLight = false;
    bool isRoom = false;

    std::string name;
    glm::vec3 position{ 0.0f };
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> textures;
    std::vector<unsigned int> indices;
    
    glm::vec3 materialAmbient{ 1.0f };
    glm::vec3 materialDiffuse{ 1.0f };
    glm::vec3 materialSpecular{ 1.0f };
    float materialShininess = 32.0f;

    glm::vec3 lightDiffuse = glm::vec3(1.0f);  // Luz difusa (Ld)
    glm::vec3 lightSpecular = glm::vec3(1.0f); // Luz especular (Ls)
    glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f); // Atenuacion ambiental 
    float w = 1.0f; // 1: puntual, 0: direccional
    int lightingType = 0; // 0: Lambert, 1: Phong, 2: Blinn-Phong

    glm::vec3 aabbMin;
    glm::vec3 aabbMax;  
};

class Scene {
public:
    Scene();
    ~Scene();
    void Render(Camera& camera);
    bool RayAABBIntersection(glm::vec3 rayStart, glm::vec3 rayDir, const Model& model, float& tNear, float& tFar);
    void CalculateAABB(Model& model, bool isLight);
    void SaveScene(const std::string& filename);
    void LoadScene(const std::string& filename);
    void generateSphere(float radius, int stacks, int slices, std::vector<float>& vertices, std::vector<unsigned int>& indices);
    void SetupModelBuffers(Model& model, const std::vector<float>& vertexData);
   
    glm::vec3 lightPosition{ 0.0f };
    glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f); // Luz ambiental (La) definida

    std::vector<Model> models;
    Model* selectedModel = nullptr;

    std::vector<Model> lightSpheres; 
    std::vector<glm::vec3> lightPositions;
    int activeLights = 1;
    std::vector<Model> staticModels;
       
private:
    Shader shader;
    Shader crosshairShader;
    Shader bboxShader;

    GLuint crosshairVAO = 0, crosshairVBO = 0;   
    GLuint bboxVAO = 0, bboxVBO = 0;
    
    void LoadOBJ(const std::string& path, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures, std::vector<unsigned int>& indices);
    void NormalizeModel(Model& model, std::vector<float>& vertices);   
    void CreateLightSphere();
    void RenderModel(const Model& model, bool isLight);
    void DrawBoundingBox(const Model& model, const Camera& camera);
    void LoadRoom();
};