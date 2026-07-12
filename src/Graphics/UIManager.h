#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../Scene/Scene.h"

class UIManager {
public:
    void Init(GLFWwindow* window);
    void NewFrame();
    void Render(Scene* scene);
    void Shutdown();
};