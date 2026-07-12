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
    ImGui::Begin("Panel de Control");

    if (scene && scene->selectedModel) {
        ImGui::Text("Modelo Seleccionado: %s", scene->selectedModel->name.c_str());
        ImGui::Separator();

        ImGui::Text("Transformaciones");
        
        // Controladores para la posicion y escala
        ImGui::DragFloat3("Posicion", &scene->selectedModel->position[0], 0.05f);
        ImGui::DragFloat3("Escala", &scene->selectedModel->scale[0], 0.05f);

        // Actualizar el Bounding Box en tiempo real si se modifica algo en la UI
        if (ImGui::IsItemEdited() || ImGui::IsItemActive()) {
            scene->selectedModel->CalculateAABB();
        }

        if (scene->selectedModel->isLight) {
            ImGui::Separator();
            ImGui::Text("Propiedades de Luz");
            ImGui::ColorEdit3("Luz Difusa", &scene->selectedModel->lightDiffuse[0]);
            ImGui::ColorEdit3("Luz Especular", &scene->selectedModel->lightSpecular[0]);
            ImGui::DragFloat3("Atenuacion", &scene->selectedModel->attenuation[0], 0.01f, 0.0f, 10.0f);
        }
    } else {
        ImGui::Text("Haz clic en un modelo para editarlo.");
    }

    ImGui::End();

    // Renderizar los datos recopilados por ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}