#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "../Scene/Scene.h"

class UIManager {
public:
    UIManager() : notificationTimer(0.0f) {}
    void Init(GLFWwindow* window);
    void NewFrame();
    void Render(Scene* scene);
    void Shutdown();
    void ShowNotification(const std::string& message);

private:
    float notificationTimer;
    std::string notificationMessage;
};