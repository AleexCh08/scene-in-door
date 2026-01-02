#include "Scene.h"
#include <cmath>
#include <fstream>
#include <map>
#include <tuple>
#include <sstream>
#include <nlohmann/json.hpp>

Scene::Scene() : shader(true), crosshairShader(crosshairVertexShader, crosshairFragmentShader), 
                    bboxShader(bboxVertexShader, bboxFragmentShader) {
    LoadRoom(); // Cargar habitacion
    // Cargar modelos desde JSON
    std::ifstream file("objects.json");
    if (!file.is_open()) {
        std::cerr << "Error al abrir objects.json" << std::endl;
        return;
    }

    nlohmann::json jsonData;
    file >> jsonData;

    for (const auto& modelData : jsonData["models"]) {
        Model model;
        model.name = modelData["name"].get<std::string>();
        model.position = glm::vec3(
            modelData["position"][0].get<float>(),
            modelData["position"][1].get<float>(),
            modelData["position"][2].get<float>()
        );

        LoadOBJ(modelData["name"].get<std::string>(), model.vertices, model.normals, model.textures, model.indices);            
        NormalizeModel(model, model.vertices);
        CalculateAABB(model, false);

        // Preparar vertexData
        std::vector<float> vertexData;
        for (size_t i = 0; i < model.vertices.size() / 3; ++i) {
            // Posici�n
            vertexData.insert(vertexData.end(), {
                model.vertices[i * 3], model.vertices[i * 3 + 1], model.vertices[i * 3 + 2]
                });

            // Normales
            vertexData.insert(vertexData.end(), {
                model.normals[i * 3], model.normals[i * 3 + 1], model.normals[i * 3 + 2]
                });

            // Texturas
            vertexData.insert(vertexData.end(), {
                model.textures[i * 2], model.textures[i * 2 + 1]
                });
        }

        SetupModelBuffers(model, vertexData);

        // Material
        model.materialAmbient = glm::vec3(
            modelData["material"]["ambient"][0].get<float>(),
            modelData["material"]["ambient"][1].get<float>(),
            modelData["material"]["ambient"][2].get<float>()
        );
        model.materialDiffuse = glm::vec3(
            modelData["material"]["diffuse"][0].get<float>(),
            modelData["material"]["diffuse"][1].get<float>(),
            modelData["material"]["diffuse"][2].get<float>()
        );
        model.materialSpecular = glm::vec3(
            modelData["material"]["specular"][0].get<float>(),
            modelData["material"]["specular"][1].get<float>(),
            modelData["material"]["specular"][2].get<float>()
        );
        model.materialShininess = modelData["material"]["shininess"].get<float>();

        models.push_back(model);
    }

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

void Scene::LoadOBJ(const std::string& path, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures, std::vector<unsigned int>& indices) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo OBJ: " << path << std::endl;
        return;
    }

    std::vector<glm::vec3> tempVertices;  // Almacena los v�rtices (v)
    std::vector<glm::vec3> tempNormals;   // Almacena las normales (vn)
    std::vector<glm::vec2> tempTextures;  // Almacena las coordenadas de textura (vt)

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;  // �ndices de las caras (f)

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {  // V�rtices
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            tempVertices.push_back(vertex);
        }
        else if (type == "vt") {  // Coordenadas de textura
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            tempTextures.push_back(uv);
        }
        else if (type == "vn") {  // Normales
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (type == "f") {  // Caras
            std::vector<unsigned int> faceVertexIndices, faceUvIndices, faceNormalIndices;

            std::string faceData;
            while (iss >> faceData) {
                std::istringstream faceStream(faceData);
                std::string vertexInfo;
                unsigned int vertexIndex = 0, uvIndex = 0, normalIndex = 0;

                // Leer el �ndice del v�rtice
                std::getline(faceStream, vertexInfo, '/');
                if (!vertexInfo.empty()) vertexIndex = std::stoi(vertexInfo);

                // Leer el �ndice de la textura
                std::getline(faceStream, vertexInfo, '/');
                if (!vertexInfo.empty()) uvIndex = std::stoi(vertexInfo);

                // Leer el �ndice de la normal
                std::getline(faceStream, vertexInfo, '/');
                if (!vertexInfo.empty()) normalIndex = std::stoi(vertexInfo);

                // Almacenar los �ndices de la cara
                if (vertexIndex > 0) faceVertexIndices.push_back(vertexIndex - 1);  // Convertir a base 0
                if (uvIndex > 0) faceUvIndices.push_back(uvIndex - 1);              // Convertir a base 0
                if (normalIndex > 0) faceNormalIndices.push_back(normalIndex - 1);  // Convertir a base 0
            }

            if (faceVertexIndices.size() > 3) {
                for (size_t i = 1; i < faceVertexIndices.size() - 1; ++i) {
                    vertexIndices.push_back(faceVertexIndices[0]);
                    vertexIndices.push_back(faceVertexIndices[i]);
                    vertexIndices.push_back(faceVertexIndices[i + 1]);

                    if (!faceUvIndices.empty()) {
                        uvIndices.push_back(faceUvIndices[0]);
                        uvIndices.push_back(faceUvIndices[i]);
                        uvIndices.push_back(faceUvIndices[i + 1]);
                    }

                    if (!faceNormalIndices.empty()) {
                        normalIndices.push_back(faceNormalIndices[0]);
                        normalIndices.push_back(faceNormalIndices[i]);
                        normalIndices.push_back(faceNormalIndices[i + 1]);
                    }
                }
            }
            else if (faceVertexIndices.size() == 3) {
                vertexIndices.insert(vertexIndices.end(), faceVertexIndices.begin(), faceVertexIndices.end());
                if (!faceUvIndices.empty()) uvIndices.insert(uvIndices.end(), faceUvIndices.begin(), faceUvIndices.end());
                if (!faceNormalIndices.empty()) normalIndices.insert(normalIndices.end(), faceNormalIndices.begin(), faceNormalIndices.end());
            }
        }
    }

    // Procesar los �ndices para generar los datos finales
    std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> uniqueVertices;  // Para evitar duplicados

    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = (i < uvIndices.size()) ? uvIndices[i] : 0;
        unsigned int normalIndex = (i < normalIndices.size()) ? normalIndices[i] : 0;

        // Crear una clave �nica para el v�rtice (v, vt, vn)
        auto key = std::make_tuple(vertexIndex, uvIndex, normalIndex);

        // Si el v�rtice no existe, a�adirlo a los datos finales
        if (uniqueVertices.find(key) == uniqueVertices.end()) {
            uniqueVertices[key] = static_cast<unsigned int>(vertices.size() / 3);

            // A�adir v�rtice
            vertices.push_back(tempVertices[vertexIndex].x);
            vertices.push_back(tempVertices[vertexIndex].y);
            vertices.push_back(tempVertices[vertexIndex].z);

            // A�adir textura (si existe)
            if (!tempTextures.empty() && uvIndex < tempTextures.size()) {
                textures.push_back(tempTextures[uvIndex].x);
                textures.push_back(tempTextures[uvIndex].y);
            }
            else {
                textures.push_back(0.0f);
                textures.push_back(0.0f);
            }

            // A�adir normal (si existe)
            if (!tempNormals.empty() && normalIndex < tempNormals.size()) {
                normals.push_back(tempNormals[normalIndex].x);
                normals.push_back(tempNormals[normalIndex].y);
                normals.push_back(tempNormals[normalIndex].z);
            }
            else {
                normals.push_back(0.0f);
                normals.push_back(0.0f);
                normals.push_back(0.0f);
            }
        }

        // A�adir el �ndice del v�rtice
        indices.push_back(uniqueVertices[key]);
    }
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
}

void Scene::RenderModel(const Model& model, bool isLight) {   
    shader.SetBool("isLightSource", isLight);

    if (!isLight) {
        shader.SetVec3("material.ambient", model.materialAmbient);
        shader.SetVec3("material.diffuse", model.materialDiffuse);
        shader.SetVec3("material.specular", model.materialSpecular);
        shader.SetFloat("material.shininess", model.materialShininess);
    }
    else {
        shader.SetVec3("lightColor", model.lightDiffuse);
    }

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), model.position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, model.scale);

    shader.SetMat4("model", modelMatrix);

    glBindVertexArray(model.VAO);
    glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Scene::generateSphere(float radius, int stacks, int slices, std::vector<float>& vertices, std::vector<unsigned int>& indices) 
{
    const float pi = 3.14159265358979323846f;

    // Generar v�rtices
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = pi / 2 - i * (pi / stacks);
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= slices; ++j) {
            float sliceAngle = j * (2 * pi / slices);
            float x = xy * cosf(sliceAngle);
            float y = xy * sinf(sliceAngle);

            // Posici�n (x, y, z)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normales (normalizadas)
            vertices.push_back(x / radius); // nx
            vertices.push_back(y / radius); // ny
            vertices.push_back(z / radius); // nz

            // Coordenadas de textura
            float s = (float)j / slices;
            float t = (float)i / stacks;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    // Generar �ndices
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
}

void Scene::CreateLightSphere() {
    lightSpheres.resize(10); // Reservar espacio para 10 luces
    float spacing = 0.5f;
    
    for (int i = 0; i < 10; i++) {     
        Model lightSphere;
        lightSphere.isLight = true;
        lightSphere.position = glm::vec3(0.0f, 2.0f, 0.0f);
        float radius = 0.1f;
        generateSphere(radius, 20, 20, lightSphere.vertices, lightSphere.indices);

        std::vector<float> vertexData;
        for (size_t j = 0; j < lightSphere.vertices.size() / 8; ++j) {
            for (int k = 0; k < 8; ++k) {
                vertexData.push_back(lightSphere.vertices[j * 8 + k]);
            }
        }

        SetupModelBuffers(lightSphere, vertexData);
        lightSphere.materialAmbient = glm::vec3(1.0f);
        lightSphere.materialDiffuse = glm::vec3(1.0f);
        lightSphere.materialSpecular = glm::vec3(1.0f);
        lightSphere.materialShininess = 32.0f;

        lightSphere.lightDiffuse = glm::vec3(1.0f); // Color difuso
        lightSphere.lightSpecular = glm::vec3(1.0f); // Color especular
        lightSphere.attenuation = glm::vec3(1.0f, 0.0f, 0.0f); // a=1, b=0, c=0 (sin atenuaci�n)
        lightSphere.w = 1.0f; // Puntual por defecto
        lightSphere.lightingType = 0; // Lambert por defecto

        CalculateAABB(lightSphere, true);
        lightSpheres[i] = lightSphere;    
    }
}

void Scene::CalculateAABB(Model& model, bool isLight) {
    if (model.vertices.empty()) return;

    model.aabbMin = glm::vec3(FLT_MAX);
    model.aabbMax = glm::vec3(-FLT_MAX);

    size_t stride = isLight ? 8 : 3;

    for (size_t i = 0; i < model.vertices.size(); i += stride) {
        glm::vec3 vertexLocal(
            model.vertices[i] * model.scale.x,
            model.vertices[i + 1] * model.scale.y,
            model.vertices[i + 2] * model.scale.z
        );

        model.aabbMin = glm::min(model.aabbMin, vertexLocal);
        model.aabbMax = glm::max(model.aabbMax, vertexLocal);
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
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    bboxShader.SetMat4("model", modelMatrix);
    bboxShader.SetMat4("view", view);
    bboxShader.SetMat4("projection", projection);
    bboxShader.SetVec3("color", glm::vec3(1.0f, 1.0f, 0.0f)); // Color amarillo

    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

void Scene::NormalizeModel(Model& model, std::vector<float>& vertices) {
    if (vertices.empty()) return;

    float minX = vertices[0], maxX = vertices[0];
    float minY = vertices[1], maxY = vertices[1];
    float minZ = vertices[2], maxZ = vertices[2];

    for (size_t i = 0; i < vertices.size(); i += 3) {
        float x = vertices[i];
        float y = vertices[i + 1];
        float z = vertices[i + 2];

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
        minZ = std::min(minZ, z);
        maxZ = std::max(maxZ, z);
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;

    float width = maxX - minX;
    float height = maxY - minY;
    float depth = maxZ - minZ;
    float maxDimension = std::max({ width, height, depth });

    if (maxDimension == 0.0f) return;

    float scale = 1.0f / maxDimension;

    for (size_t i = 0; i < vertices.size(); i += 3) {
        vertices[i] = (vertices[i] - centerX) * scale;
        vertices[i + 1] = (vertices[i + 1] - centerY) * scale;
        vertices[i + 2] = (vertices[i + 2] - centerZ) * scale;
    }
}

void Scene::LoadRoom() {
    Model room;
    room.isRoom = true;
 
    LoadOBJ("objects/room.obj", room.vertices, room.normals, room.textures, room.indices); 
    room.position = glm::vec3(0.0f, -1.0f, 0.0f);
    room.scale = glm::vec3(1.5f);
    room.rotation = glm::vec3(0.0f);
    CalculateAABB(room, false);

    // Preparar vertexData
    std::vector<float> vertexData;
    for (size_t i = 0; i < room.vertices.size() / 3; ++i) {
        // Posici�n
        vertexData.insert(vertexData.end(), {
            room.vertices[i * 3], room.vertices[i * 3 + 1], room.vertices[i * 3 + 2]
            });

        // Normales
        vertexData.insert(vertexData.end(), {
            room.normals[i * 3], room.normals[i * 3 + 1], room.normals[i * 3 + 2]
            });

        // Texturas
        vertexData.insert(vertexData.end(), {
            room.textures[i * 2], room.textures[i * 2 + 1]
            });
    }
    // Configurar buffers y a�adir a la escena
    SetupModelBuffers(room, vertexData);

    // Material de la habitaci�n
    room.materialAmbient = glm::vec3(0.3, 0.3, 0.3);
    room.materialDiffuse = glm::vec3(0.5, 0.45, 0.4);
    room.materialSpecular = glm::vec3(0.7, 0.7, 0.65);
    room.materialShininess = 80.0f;

    models.push_back(room);
}

void Scene::SaveScene(const std::string& filename) {
    nlohmann::json jsonData;

    // Guardar modelos regulares (excluyendo la habitaci�n)
    for (const auto& model : models) {
        if (model.isRoom) continue;

        nlohmann::json modelJson;
        modelJson["name"] = model.name;
        modelJson["position"] = { model.position.x, model.position.y, model.position.z };
        modelJson["rotation"] = { model.rotation.x, model.rotation.y, model.rotation.z, model.rotation.w };
        modelJson["scale"] = { model.scale.x, model.scale.y, model.scale.z };
        modelJson["material"]["ambient"] = { model.materialAmbient.x, model.materialAmbient.y, model.materialAmbient.z };
        modelJson["material"]["diffuse"] = { model.materialDiffuse.x, model.materialDiffuse.y, model.materialDiffuse.z };
        modelJson["material"]["specular"] = { model.materialSpecular.x, model.materialSpecular.y, model.materialSpecular.z };
        modelJson["material"]["shininess"] = model.materialShininess;
        modelJson["isLight"] = false;

        jsonData["models"].push_back(modelJson);
    }

    // Guardar luces activas
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
        file << jsonData.dump(4); // Formato legible
        file.close();
    }
    else {
        std::cerr << "Error al guardar la escena en " << filename << std::endl;
    }
}

void Scene::LoadScene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al cargar la escena desde " << filename << std::endl;
        return;
    }

    nlohmann::json jsonData;
    file >> jsonData;

    // Limpiar modelos existentes (excepto la habitaci�n)
    models.erase(std::remove_if(models.begin(), models.end(), [](const Model& m) { return !m.isRoom; }), models.end());

    // Reiniciar luces
    activeLights = 0;
    for (auto& light : lightSpheres) {
        light = Model(); // Resetear a valores por defecto
    }

    for (const auto& modelJson : jsonData["models"]) {
        if (modelJson["isLight"].get<bool>()) {
            // Cargar como luz
            if (activeLights >= 10) {
                std::cerr << "L�mite de luces alcanzado (10)" << std::endl;
                continue;
            }

            Model& light = lightSpheres[activeLights];
            light.position = glm::vec3(
                modelJson["position"][0].get<float>(),
                modelJson["position"][1].get<float>(),
                modelJson["position"][2].get<float>()
            );
            light.lightDiffuse = glm::vec3(
                modelJson["lightDiffuse"][0].get<float>(),
                modelJson["lightDiffuse"][1].get<float>(),
                modelJson["lightDiffuse"][2].get<float>()
            );
            light.lightSpecular = glm::vec3(
                modelJson["lightSpecular"][0].get<float>(),
                modelJson["lightSpecular"][1].get<float>(),
                modelJson["lightSpecular"][2].get<float>()
            );
            light.attenuation = glm::vec3(
                modelJson["attenuation"][0].get<float>(),
                modelJson["attenuation"][1].get<float>(),
                modelJson["attenuation"][2].get<float>()
            );
            light.w = modelJson["w"].get<float>();
            light.lightingType = modelJson["lightingType"].get<int>();
            light.isLight = true;

            // Regenerar geometr�a de la esfera
            generateSphere(0.1f, 20, 20, light.vertices, light.indices);
            std::vector<float> vertexData;
            for (size_t j = 0; j < light.vertices.size() / 8; ++j) {
                for (int k = 0; k < 8; ++k) {
                    vertexData.push_back(light.vertices[j * 8 + k]);
                }
            }
            SetupModelBuffers(light, vertexData);           
            CalculateAABB(light, true);
            activeLights++;
        }
        else {
            // Cargar modelo desde OBJ
            Model model;
            model.name = modelJson["name"].get<std::string>();
            model.position = glm::vec3(
                modelJson["position"][0].get<float>(),
                modelJson["position"][1].get<float>(),
                modelJson["position"][2].get<float>()
            );
            model.rotation = glm::quat(
                modelJson["rotation"][3].get<float>(),
                modelJson["rotation"][0].get<float>(),
                modelJson["rotation"][1].get<float>(),
                modelJson["rotation"][2].get<float>() 
            );
            model.scale = glm::vec3(
                modelJson["scale"][0].get<float>(),
                modelJson["scale"][1].get<float>(),
                modelJson["scale"][2].get<float>()
            );
            model.materialAmbient = glm::vec3(
                modelJson["material"]["ambient"][0].get<float>(),
                modelJson["material"]["ambient"][1].get<float>(),
                modelJson["material"]["ambient"][2].get<float>()
            );
            model.materialDiffuse = glm::vec3(
                modelJson["material"]["diffuse"][0].get<float>(),
                modelJson["material"]["diffuse"][1].get<float>(),
                modelJson["material"]["diffuse"][2].get<float>()
            );
            model.materialSpecular = glm::vec3(
                modelJson["material"]["specular"][0].get<float>(),
                modelJson["material"]["specular"][1].get<float>(),
                modelJson["material"]["specular"][2].get<float>()
            );
            model.materialShininess = modelJson["material"]["shininess"].get<float>();
            model.isLight = false;

            // Cargar OBJ y configurar buffers
            LoadOBJ(model.name, model.vertices, model.normals, model.textures, model.indices);
            NormalizeModel(model, model.vertices);
            CalculateAABB(model, false);
            // Preparar vertexData
            std::vector<float> vertexData;
            for (size_t i = 0; i < model.vertices.size() / 3; ++i) {
                // Posici�n
                vertexData.insert(vertexData.end(), {
                    model.vertices[i * 3], model.vertices[i * 3 + 1], model.vertices[i * 3 + 2]
                    });

                // Normales
                vertexData.insert(vertexData.end(), {
                    model.normals[i * 3], model.normals[i * 3 + 1], model.normals[i * 3 + 2]
                    });

                // Texturas
                vertexData.insert(vertexData.end(), {
                    model.textures[i * 2], model.textures[i * 2 + 1]
                    });
            }
            SetupModelBuffers(model, vertexData);
            models.push_back(model);
        }
    }
    selectedModel = nullptr; // Deseleccionar al cargar
}

void Scene::SetupModelBuffers(Model& model, const std::vector<float>& vertexData) {
    glGenVertexArrays(1, &model.VAO);
    glGenBuffers(1, &model.VBO);
    glGenBuffers(1, &model.EBO);

    glBindVertexArray(model.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int), model.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Scene::~Scene() {
    // Liberar VAO, VBO y EBO de los modelos
    for (auto& model : models) {
        glDeleteVertexArrays(1, &model.VAO);
        glDeleteBuffers(1, &model.VBO);
        glDeleteBuffers(1, &model.EBO);
    }

    // Liberar VAO y VBO de la mira
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);

    // Liberar VAO y VBO del bounding box
    glDeleteVertexArrays(1, &bboxVAO);
    glDeleteBuffers(1, &bboxVBO);
}