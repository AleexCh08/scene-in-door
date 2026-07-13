#pragma once
#include "../Graphics/Shader.h"
#include "../Core/Camera.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../Graphics/Model.h"
#include "../Graphics/Skybox.h"

class Scene {
public:
    Scene();
    ~Scene();
    Skybox skybox;
    void Render(Camera& camera);

    Shader pickingShader;
    unsigned int pickingFBO;
    unsigned int pickingTexture;
    unsigned int pickingDepth;
    
    void InitPickingFBO(int width, int height);
    void RenderPickingPass(Camera& camera);

    bool RayAABBIntersection(glm::vec3 rayStart, glm::vec3 rayDir, const Model& model, float& tNear, float& tFar);
    void SaveScene(const std::string& filename, Camera& camera);
    void LoadScene(const std::string& filename, Camera& camera);
   
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
       
    void CreateLightSphere();
    void RenderModel(const Model& model, bool isLight);
    void DrawBoundingBox(const Model& model, const Camera& camera);
};