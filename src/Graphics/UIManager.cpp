#include "UIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void UIManager::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    // Inicializar los backends de ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void UIManager::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::Render(Scene* scene) {
    // Heurística 8: Diseño estético y minimalista. Solo mostramos la UI si hay selección.
    if (scene && scene->selectedModel) {
        ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_Always);
        ImGui::Begin("Inspector de Propiedades", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        // Heurística 1: Visibilidad del estado del sistema
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Seleccion actual:");
        if (scene->selectedModel->isLight) {
            int lightIndex = -1;
            for (int i = 0; i < scene->activeLights; i++) {
                if (scene->selectedModel == &scene->lightSpheres[i]) {
                    lightIndex = i + 1;
                    break;
                }
            }
            ImGui::TextWrapped("Luz %d", lightIndex);
        } else {
            ImGui::TextWrapped("%s", scene->selectedModel->name.c_str());
        }
        ImGui::Spacing();

        // Heurística 3: Control y libertad del usuario (Salida clara)
        if (ImGui::Button("Deseleccionar", ImVec2(-1, 30))) {
            scene->selectedModel = nullptr;
            ImGui::End();
            
            // ImGui requiere terminar el ciclo de renderizado aunque cerremos la ventana
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            return; 
        }
        
        ImGui::Separator();
        ImGui::Spacing();

        // Heurística 4: Consistencia y estándares (Agrupación lógica)
        if (ImGui::CollapsingHeader("Transformaciones", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Posicion", &scene->selectedModel->position[0], 0.05f);
            // Las luces no se escalan ni rotan
            if (!scene->selectedModel->isLight) {
                ImGui::DragFloat3("Escala", &scene->selectedModel->scale[0], 0.05f);
                
                glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(scene->selectedModel->rotation));
                if (ImGui::DragFloat3("Rotacion", &eulerRotation[0], 1.0f)) {
                    scene->selectedModel->rotation = glm::quat(glm::radians(eulerRotation));
                }
            }
            
            if (ImGui::IsItemEdited() || ImGui::IsItemActive()) {
                scene->selectedModel->CalculateAABB();
            }
        }

        if (scene->selectedModel->isLight) {
            if (ImGui::CollapsingHeader("Propiedades de Luz", ImGuiTreeNodeFlags_DefaultOpen)) {
                
                ImGui::Text("Modelo de Iluminacion:");
                ImGui::RadioButton("Lambert", &scene->selectedModel->lightingType, 0); ImGui::SameLine();
                ImGui::RadioButton("Phong", &scene->selectedModel->lightingType, 1); ImGui::SameLine();
                ImGui::RadioButton("Blinn-Phong", &scene->selectedModel->lightingType, 2);

                ImGui::Spacing();
                ImGui::Text("Tipo de Luz:");
                bool isPointLight = (scene->selectedModel->w == 1.0f);
                if (ImGui::Checkbox("Luz Puntual", &isPointLight)) {
                    scene->selectedModel->w = isPointLight ? 1.0f : 0.0f;
                }
                
                ImGui::Separator();
                ImGui::ColorEdit3("Luz Difusa", &scene->selectedModel->lightDiffuse[0]);
                ImGui::ColorEdit3("Luz Especular", &scene->selectedModel->lightSpecular[0]);
                
                // La atenuacion solo es relevante si la luz es puntual
                if (isPointLight) {
                    ImGui::DragFloat3("Atenuacion", &scene->selectedModel->attenuation[0], 0.01f, 0.0f, 10.0f);
                }
            }
            
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::CollapsingHeader("Gestor de Escena", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Luces Activas: %d / 10", scene->activeLights);
                
                if (scene->activeLights < 10) {
                    if (ImGui::Button("Añadir Luz")) {
                        int prevLights = scene->activeLights;
                        scene->activeLights++;
                        scene->lightSpheres[prevLights] = scene->lightSpheres[0]; 
                        scene->lightSpheres[prevLights].position = scene->lightSpheres[prevLights - 1].position + glm::vec3(0.25f, 0.0f, 0.0f);
                        scene->lightSpheres[prevLights].CalculateAABB();
                    }
                }
                
                if (scene->activeLights > 1) {
                    ImGui::SameLine();
                    if (ImGui::Button("Quitar Luz")) {
                        if (scene->selectedModel == &scene->lightSpheres[scene->activeLights - 1]) {
                            scene->selectedModel = nullptr;
                            scene->activeLights--;
                            ImGui::End();
                            ImGui::Render();
                            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                            return; 
                        } else {
                            scene->activeLights--;
                        }
                    }
                }
            }
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}