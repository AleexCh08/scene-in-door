#include "UIManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>
#include "../Utils/tinyfiledialogs.h"

void UIManager::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Estilo visual moderno y profesional
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Redondeo de bordes
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    
    // Espaciado y alineacion
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // Centrar el titulo de la ventana

    // Paleta de colores (Tonos oscuros con acentos azules/grises)
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.13f, 0.95f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.26f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.31f, 0.32f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.22f, 0.33f, 0.55f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.39f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.45f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.32f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.40f, 0.45f, 1.00f);
    
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
    if (scene && scene->selectedModel) {
        // AlwaysAutoResize acopla la ventana exactamente al tamaño de sus elementos
        ImGui::Begin("Inspector de Propiedades", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

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
            std::string cleanName = std::filesystem::path(scene->selectedModel->name).stem().string();
            ImGui::TextWrapped("%s", cleanName.c_str());
        }
        ImGui::Spacing();

        if (scene->selectedModel->isLight) {
            ImVec2 deseleccionarSize(160, 35);
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - deseleccionarSize.x) * 0.5f);
            
            if (ImGui::Button("Deseleccionar", deseleccionarSize)) {
                scene->selectedModel = nullptr;
                ImGui::End();
                
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                return; 
            }
        } 
        // Si es un modelo, mostramos Deseleccionar y Eliminar lado a lado
        else {
            // Hacemos los botones un poco más pequeños (120) para que quepan cómodamente
            ImVec2 buttonSize(120, 35); 
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            
            // El ancho total del grupo es: anchoBoton * 2 + espacioEntreBotones
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - (buttonSize.x * 2 + spacing)) * 0.5f);
            
            if (ImGui::Button("Deseleccionar", buttonSize)) {
                scene->selectedModel = nullptr;
                ImGui::End();
                
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                return; 
            }
            
            ImGui::SameLine();
            
            // Boton Eliminar con los mismos tonos rojos que el boton de Salir
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
            
            if (ImGui::Button("Eliminar", buttonSize)) {
                int targetID = scene->selectedModel->pickingID;
                
                // Buscar el modelo por su ID único en el vector y borrarlo
                for (auto it = scene->models.begin(); it != scene->models.end(); ++it) {
                    if (it->pickingID == targetID) {
                        scene->models.erase(it);
                        break;
                    }
                }
                
                // Limpiar la seleccion para que no apunte a un bloque de memoria destruido
                scene->selectedModel = nullptr;
                
                ImGui::PopStyleColor(3);
                ImGui::End();
                
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                return; 
            }
            
            ImGui::PopStyleColor(3);
        }
        
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Transformaciones", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(10.0f); 
            bool transformChanged = false;
            auto DrawVec3Control = [&](const std::string& label, glm::vec3& values, float speed = 0.05f, float min = 0.0f, float max = 0.0f) -> bool {
                bool changed = false;
                ImGui::Text("%s", label.c_str()); 
                ImGui::SameLine(85.0f); 
                
                float width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemInnerSpacing.x * 2.0f) / 3.0f;
                float dragWidth = width - ImGui::CalcTextSize("X").x - 4.0f; 

                auto DrawColoredDrag = [&](const char* id, const char* labelStr, ImVec4 color, float& val) {
                    ImGui::TextColored(color, "%s", labelStr);
                    ImGui::SameLine(0, 4.0f);
                    ImGui::PushItemWidth(dragWidth);
                    if (ImGui::DragFloat(id, &val, speed, min, max, "%.2f")) changed = true;
                    ImGui::PopItemWidth();
                };

                DrawColoredDrag((std::string("##X") + label).c_str(), "X", ImVec4(1.0f, 0.3f, 0.3f, 1.0f), values.x);
                ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
                DrawColoredDrag((std::string("##Y") + label).c_str(), "Y", ImVec4(0.3f, 1.0f, 0.3f, 1.0f), values.y);
                ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
                DrawColoredDrag((std::string("##Z") + label).c_str(), "Z", ImVec4(0.3f, 0.6f, 1.0f, 1.0f), values.z);
                
                return changed;
            };

            // Usamos la lambda para cada transformacion
            if (DrawVec3Control("Posicion", scene->selectedModel->position)) transformChanged = true;
            
            if (!scene->selectedModel->isLight) {
                if (DrawVec3Control("Escala", scene->selectedModel->scale, 0.05f, 0.01f, 100.0f)) transformChanged = true;
                
                glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(scene->selectedModel->rotation));
                if (DrawVec3Control("Rotacion", eulerRotation, 1.0f)) {
                    scene->selectedModel->rotation = glm::quat(glm::radians(eulerRotation));
                    transformChanged = true;
                }
            }
            
            // Si hubo cambio en posicion, escala o rotacion, recalculamos el Bounding Box
            if (transformChanged) {
                scene->selectedModel->CalculateAABB();
            }
            
            ImGui::Unindent(10.0f);
        }

        if (scene->selectedModel->isLight) {
            if (ImGui::CollapsingHeader("Propiedades de Luz", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Modelo de Iluminacion:");
                ImGui::RadioButton("Lambert", &scene->selectedModel->lightingType, 0); ImGui::SameLine();
                ImGui::RadioButton("Phong", &scene->selectedModel->lightingType, 1); ImGui::SameLine();
                ImGui::RadioButton("Blinn-Phong", &scene->selectedModel->lightingType, 2);

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Tipo de Luz:");
                bool isPointLight = (scene->selectedModel->w == 1.0f);
                if (ImGui::Checkbox("Luz Puntual", &isPointLight)) {
                    scene->selectedModel->w = isPointLight ? 1.0f : 0.0f;
                }
                
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::ColorEdit3("Luz Difusa", &scene->selectedModel->lightDiffuse[0]);
                ImGui::ColorEdit3("Luz Especular", &scene->selectedModel->lightSpecular[0]);
                
                if (isPointLight) {
                    ImGui::DragFloat3("Atenuacion", &scene->selectedModel->attenuation[0], 0.01f, 0.0f, 10.0f);
                }
                ImGui::Unindent(10.0f);
            }
            
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::CollapsingHeader("Gestor de Escena", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.8f, 1.0f), "Luces Activas: %d / 10", scene->activeLights);
                ImGui::Spacing();
                
                if (scene->activeLights < 10) {
                    if (ImGui::Button(" Añadir Luz ")) {
                        int prevLights = scene->activeLights;
                        scene->activeLights++;
                        scene->lightSpheres[prevLights] = scene->lightSpheres[0]; 
                        scene->lightSpheres[prevLights].position = scene->lightSpheres[prevLights - 1].position + glm::vec3(0.25f, 0.0f, 0.0f);
                        scene->lightSpheres[prevLights].CalculateAABB();
                    }
                }
                
                if (scene->activeLights > 1) {
                    if (scene->activeLights < 10) ImGui::SameLine(); 
                    if (ImGui::Button(" Quitar Luz ")) {
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
                ImGui::Unindent(10.0f);
            }
        }
        
        ImGui::End();       
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    
    // Aumentamos el margen interno para que no se vea aplastada
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));
    ImGui::Begin("Menu Global (P)", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    
    // Asignamos un tamaño fijo a los botones (Ancho, Alto) para forzar el tamaño de la ventana
    ImVec2 buttonSize(160, 35);

    if (ImGui::Button("Importar Modelo 3D", buttonSize)) {
        if (scene) {
            const char* filterPatterns[1] = { "*.obj" };
            const char* filepath = tinyfd_openFileDialog("Importar Modelo 3D", "", 1, filterPatterns, "Modelos 3D", 0);
            
            if (filepath) {
                    int maxID = 10;
                    for (const auto& m : scene->models) {
                        if (m.pickingID > maxID) maxID = m.pickingID;
                    }
                    
                    scene->models.emplace_back(filepath);
                    scene->models.back().SetPickingID(maxID + 1);
                    
                    ShowNotification("Modelo importado exitosamente");
                }
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Guardar Escena", buttonSize)) {
        if (scene) {
            const char* filterPatterns[1] = { "*.json" };
            const char* filepath = tinyfd_saveFileDialog("Guardar Escena", "scene.json", 1, filterPatterns, "Archivos JSON");
            
            if (filepath) { 
                scene->SaveScene(filepath);
                ShowNotification("Escena guardada exitosamente");
            }
        }
    }
    ImGui::Spacing();
    
    if (ImGui::Button("Cargar Escena", buttonSize)) {
        if (scene) {
            const char* filterPatterns[1] = { "*.json" };
            const char* filepath = tinyfd_openFileDialog("Cargar Escena", "", 1, filterPatterns, "Archivos JSON", 0);
            
            if (filepath) {
                scene->selectedModel = nullptr; 
                
                scene->LoadScene(filepath);
                ShowNotification("Escena cargada exitosamente");

                ImGui::End();
                ImGui::PopStyleVar();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                return;
            }
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Coloreamos el boton de salir de rojo para diferenciarlo y darle un toque profesional
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
    
    if (ImGui::Button("Salir del Programa", buttonSize)) {
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
    }
    
    ImGui::PopStyleColor(3);
    ImGui::End();
    ImGui::PopStyleVar();

    if (notificationTimer > 0.0f) {
        ImGuiIO& io = ImGui::GetIO();
        notificationTimer -= io.DeltaTime; // Reducir tiempo en base a los FPS

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y - 50.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        // Darle un estilo verde oscuro translucido y sin bordes
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 10.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.60f, 0.25f, 0.85f)); 
        
        // Flags para que no tenga titulo, no se pueda mover y no quite el foco
        ImGui::Begin("Notificacion", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", notificationMessage.c_str());
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::ShowNotification(const std::string& message) {
    notificationMessage = message;
    notificationTimer = 3.0f; // La notificacion durara 3 segundos en pantalla
}