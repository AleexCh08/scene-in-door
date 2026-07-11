#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include "Shader.h"
#include <glm/glm.hpp>

class Skybox {
public:
    Skybox();
    ~Skybox();
    void Load(const std::vector<std::string>& faces);
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    Shader skyboxShader;
    unsigned int loadCubemap(const std::vector<std::string>& faces);
};