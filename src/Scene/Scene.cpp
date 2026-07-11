#include "Scene.h"
#include <cmath>
#include <fstream>
#include <map>
#include <tuple>
#include <sstream>
#include <nlohmann/json.hpp>

Scene::Scene() : shader(true), crosshairShader(crosshairVertexShader, crosshairFragmentShader), 
                    bboxShader(bboxVertexShader, bboxFragmentShader) {
    // Cargar modelos desde JSON
    std::ifstream file("objects.json");
    if (!file.is_open()) {
        std::cerr << "Error al abrir objects.json" << std::endl;
        return;
    }

    nlohmann::json jsonData;
    file >> jsonData;

    for (const auto& modelData : jsonData["models"]) {
        Model model(modelData["name"].get<std::string>());
        model.position = glm::vec3(
            modelData["position"][0].get<float>(),
            modelData["position"][1].get<float>(),
            modelData["position"][2].get<float>()
        );
        models.push_back(model);
    }

    // Crear skybox
    std::vector<std::string> faces = {
        "src/Textures/skybox/right.png",
        "src/Textures/skybox/left.png",
        "src/Textures/skybox/top.png",
        "src/Textures/skybox/bottom.png",
        "src/Textures/skybox/front.png",
        "src/Textures/skybox/back.png"
    };
    skybox.Load(faces);

    CreateLightSphere();

    // Configuracion de mira
    float crosshairVertices[] = {
        -0.02f, 0.0f, -0.01f, 0.0f, // L�nea horizontal izquierda
        0.01f, 0.0f, 0.02f, 0.0f,   // L�nea horizontal derecha
        0.0f, 0.01f, 0.0f, 0.02f,   // L�nea vertical superior
        0.0f, -0.02f, 0.0f, -0.01f  // L�nea vertical inferior
    };

    // Buffers para mira
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Renderizar mira
    glDisable(GL_DEPTH_TEST);
    crosshairShader.Use();
    glUniform3f(glGetUniformLocation(crosshairShader.GetID(), "color"), 1.0f, 1.0f, 1.0f);
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    // Buffers para Bounding Box
    glGenVertexArrays(1, &bboxVAO);
    glGenBuffers(1, &bboxVBO);
}


void Scene::Render(Camera& camera) 
{
    shader.Use();
    shader.SetInt("numLights", activeLights);
    shader.SetMat4("view", camera.GetViewMatrix());
    shader.SetMat4("projection", glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f));

    shader.SetVec3("viewPos", camera.Position);
    for (int i = 0; i < activeLights; ++i) {
        std::string prefix = "lights[" + std::to_string(i) + "]";
        shader.SetVec3(prefix + ".position", lightSpheres[i].position);
        shader.SetVec3(prefix + ".ambient", lightAmbient);
        shader.SetVec3(prefix + ".diffuse", lightSpheres[i].lightDiffuse); 
        shader.SetVec3(prefix + ".specular", lightSpheres[i].lightSpecular); 
        shader.SetVec3(prefix + ".attenuation", lightSpheres[i].attenuation);
        shader.SetFloat(prefix + ".w", lightSpheres[i].w);
        shader.SetInt(prefix + ".lightingType", lightSpheres[i].lightingType);
    }

    // Renderizar modelos
    for (auto& model : models) {
        RenderModel(model, false);
    }

    // Renderizar esferas de luz activas
    for (int i = 0; i < activeLights; ++i) {
        RenderModel(lightSpheres[i], true);
    }

    // Renderizar mira
    glDisable(GL_DEPTH_TEST);
    crosshairShader.Use();
    glUniform3f(glGetUniformLocation(crosshairShader.GetID(), "color"), 1.0f, 1.0f, 1.0f);
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    // Dibujar Bounding Box del objeto seleccionado
    if (selectedModel) {
        DrawBoundingBox(*selectedModel, camera);
    }

    // Renderizar skybox
    skybox.Draw(camera.GetViewMatrix(), glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f));
}

void Scene::RenderModel(const Model& model, bool isLight) {   
    shader.SetBool("isLightSource", isLight);

    if (!isLight) {
        shader.SetFloat("material.shininess", model.materialShininess);
    } else {
        shader.SetVec3("lightColor", model.lightDiffuse);
    }

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), model.position);
    // Extraemos la rotación limpia del quaternion
    glm::mat4 rotMatrix = glm::mat4_cast(model.rotation);
    modelMatrix = modelMatrix * rotMatrix;
    modelMatrix = glm::scale(modelMatrix, model.scale);

    shader.SetMat4("model", modelMatrix);
    model.Draw(shader);
}

void Scene::CreateLightSphere() {
    lightSpheres.resize(10); 
    
    // Generar vértices de la esfera adaptados a la nueva estructura Vertex
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    float radius = 0.1f;
    int stacks = 20, slices = 20;
    const float pi = 3.14159265358979323846f;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = pi / 2 - i * (pi / stacks);
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= slices; ++j) {
            float sliceAngle = j * (2 * pi / slices);
            Vertex vertex;
            vertex.Position = glm::vec3(xy * cosf(sliceAngle), xy * sinf(sliceAngle), z);
            vertex.Normal = glm::normalize(vertex.Position);
            vertex.TexCoords = glm::vec2((float)j / slices, (float)i / stacks);
            vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    std::vector<Texture> textures; // Las luces no usan texturas
    Mesh sphereMesh(vertices, indices, textures);

    for (int i = 0; i < 10; i++) {     
        Model lightSphere;
        lightSphere.isLight = true;
        lightSphere.position = glm::vec3(0.0f, 2.0f, 0.0f);
        lightSphere.meshes.push_back(sphereMesh); // Asignar la malla generada
        
        lightSphere.lightDiffuse = glm::vec3(1.0f); 
        lightSphere.lightSpecular = glm::vec3(1.0f); 
        lightSphere.attenuation = glm::vec3(1.0f, 0.0f, 0.0f); 
        lightSphere.w = 1.0f; 
        lightSphere.lightingType = 0; 
        
        lightSphere.CalculateAABB();
        lightSpheres[i] = lightSphere;    
    }
}

bool Scene::RayAABBIntersection(glm::vec3 rayStart, glm::vec3 rayDir, const Model& model, float& tNear, float& tFar) {
    glm::vec3 invDir = 1.0f / rayDir;

    glm::vec3 worldAabbMin = model.aabbMin + model.position;
    glm::vec3 worldAabbMax = model.aabbMax + model.position;

    glm::vec3 tMin = (worldAabbMin - rayStart) * invDir;
    glm::vec3 tMax = (worldAabbMax - rayStart) * invDir;

    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

    return tFar >= tNear && tNear >= 0.0f;
}

void Scene::DrawBoundingBox(const Model& model, const Camera& camera) {
    glm::vec3 aabbMinLocal = model.aabbMin;
    glm::vec3 aabbMaxLocal = model.aabbMax;
    glm::vec3 size = aabbMaxLocal - aabbMinLocal;
    glm::vec3 centerLocal = (aabbMinLocal + aabbMaxLocal) * 0.5f;

    glm::vec3 vertices[] = {
        aabbMinLocal,                                 // 0: min.x, min.y, min.z
        glm::vec3(aabbMaxLocal.x, aabbMinLocal.y, aabbMinLocal.z), // 1
        glm::vec3(aabbMaxLocal.x, aabbMinLocal.y, aabbMaxLocal.z), // 2
        glm::vec3(aabbMinLocal.x, aabbMinLocal.y, aabbMaxLocal.z), // 3
        glm::vec3(aabbMinLocal.x, aabbMaxLocal.y, aabbMinLocal.z), // 4
        glm::vec3(aabbMaxLocal.x, aabbMaxLocal.y, aabbMinLocal.z), // 5
        aabbMaxLocal,                                 // 6: max.x, max.y, max.z
        glm::vec3(aabbMinLocal.x, aabbMaxLocal.y, aabbMaxLocal.z)  // 7
    };

    float lines[] = {
        vertices[0].x, vertices[0].y, vertices[0].z, vertices[1].x, vertices[1].y, vertices[1].z,
        vertices[1].x, vertices[1].y, vertices[1].z, vertices[2].x, vertices[2].y, vertices[2].z,
        vertices[2].x, vertices[2].y, vertices[2].z, vertices[3].x, vertices[3].y, vertices[3].z,
        vertices[3].x, vertices[3].y, vertices[3].z, vertices[0].x, vertices[0].y, vertices[0].z,

        vertices[4].x, vertices[4].y, vertices[4].z, vertices[5].x, vertices[5].y, vertices[5].z,
        vertices[5].x, vertices[5].y, vertices[5].z, vertices[6].x, vertices[6].y, vertices[6].z,
        vertices[6].x, vertices[6].y, vertices[6].z, vertices[7].x, vertices[7].y, vertices[7].z,
        vertices[7].x, vertices[7].y, vertices[7].z, vertices[4].x, vertices[4].y, vertices[4].z,

        vertices[0].x, vertices[0].y, vertices[0].z, vertices[4].x, vertices[4].y, vertices[4].z,
        vertices[1].x, vertices[1].y, vertices[1].z, vertices[5].x, vertices[5].y, vertices[5].z,
        vertices[2].x, vertices[2].y, vertices[2].z, vertices[6].x, vertices[6].y, vertices[6].z,
        vertices[3].x, vertices[3].y, vertices[3].z, vertices[7].x, vertices[7].y, vertices[7].z
    };

    // Actualizar buffers
    glBindVertexArray(bboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Configurar shader
    bboxShader.Use();
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), model.position);
    /*
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));*/
    glm::mat4 rotMatrix = glm::mat4_cast(model.rotation);
    modelMatrix = modelMatrix * rotMatrix;

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    bboxShader.SetMat4("model", modelMatrix);
    bboxShader.SetMat4("view", view);
    bboxShader.SetMat4("projection", projection);
    bboxShader.SetVec3("color", glm::vec3(1.0f, 1.0f, 0.0f)); // Color amarillo

    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

void Scene::SaveScene(const std::string& filename) {
    nlohmann::json jsonData;

    for (const auto& model : models) {
        if (model.isRoom) continue;

        nlohmann::json modelJson;
        modelJson["name"] = model.name;
        modelJson["position"] = { model.position.x, model.position.y, model.position.z };
        modelJson["rotation"] = { model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w };
        modelJson["scale"] = { model.scale.x, model.scale.y, model.scale.z };
        modelJson["isLight"] = false;

        jsonData["models"].push_back(modelJson);
    }

    for (int i = 0; i < activeLights; ++i) {
        const auto& light = lightSpheres[i];
        nlohmann::json lightJson;
        lightJson["isLight"] = true;
        lightJson["position"] = { light.position.x, light.position.y, light.position.z };
        lightJson["lightDiffuse"] = { light.lightDiffuse.x, light.lightDiffuse.y, light.lightDiffuse.z };
        lightJson["lightSpecular"] = { light.lightSpecular.x, light.lightSpecular.y, light.lightSpecular.z };
        lightJson["attenuation"] = { light.attenuation.x, light.attenuation.y, light.attenuation.z };
        lightJson["w"] = light.w;
        lightJson["lightingType"] = light.lightingType;

        jsonData["models"].push_back(lightJson);
    }

    std::ofstream file(filename);
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
    }
}

void Scene::LoadScene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    nlohmann::json jsonData;
    file >> jsonData;

    models.erase(std::remove_if(models.begin(), models.end(), [](const Model& m) { return !m.isRoom; }), models.end());

    activeLights = 0;
    // IMPORTANTE: Ya no reiniciamos lightSpheres enteros para no perder la malla generada en memoria

    for (const auto& modelJson : jsonData["models"]) {
        if (modelJson.contains("isLight") && modelJson["isLight"].get<bool>()) {
            if (activeLights >= 10) continue;

            Model& light = lightSpheres[activeLights];
            light.position = glm::vec3(modelJson["position"][0].get<float>(), modelJson["position"][1].get<float>(), modelJson["position"][2].get<float>());
            light.lightDiffuse = glm::vec3(modelJson["lightDiffuse"][0].get<float>(), modelJson["lightDiffuse"][1].get<float>(), modelJson["lightDiffuse"][2].get<float>());
            light.lightSpecular = glm::vec3(modelJson["lightSpecular"][0].get<float>(), modelJson["lightSpecular"][1].get<float>(), modelJson["lightSpecular"][2].get<float>());
            light.attenuation = glm::vec3(modelJson["attenuation"][0].get<float>(), modelJson["attenuation"][1].get<float>(), modelJson["attenuation"][2].get<float>());
            light.w = modelJson["w"].get<float>();
            light.lightingType = modelJson["lightingType"].get<int>();
            
            light.CalculateAABB();
            activeLights++;
        }
        else {
            Model model(modelJson["name"].get<std::string>());
            model.position = glm::vec3(modelJson["position"][0].get<float>(), modelJson["position"][1].get<float>(), modelJson["position"][2].get<float>());
            
            if (modelJson.contains("rotation")) {
                model.rotation = glm::quat(modelJson["rotation"][3].get<float>(), modelJson["rotation"][0].get<float>(), modelJson["rotation"][1].get<float>(), modelJson["rotation"][2].get<float>());
            }
            if (modelJson.contains("scale")) {
                model.scale = glm::vec3(modelJson["scale"][0].get<float>(), modelJson["scale"][1].get<float>(), modelJson["scale"][2].get<float>());
            }
            
            model.isLight = false;
            models.push_back(model);
        }
    }
    selectedModel = nullptr;
}


Scene::~Scene() {
    // Liberar VAO y VBO de la mira
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);

    // Liberar VAO y VBO del bounding box
    glDeleteVertexArrays(1, &bboxVAO);
    glDeleteBuffers(1, &bboxVBO);
}