#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include "Core/Camera.h"
#include "Scene/Scene.h"
#include "Graphics/UIManager.h"
#include "Utils/tinyfiledialogs.h"
#include <imgui.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400, lastY = 300;
bool firstMouse = true;

bool isMousePressed = false;
bool keyXPressed = false, keyYPressed = false, keyZPressed = false;
bool keyRPressed = false, keyGPressed = false, keyBPressed = false;
bool keyJPressed = false, keyKPressed = false, keyLPressed = false;
bool keyUPressed = false, keyIPressed = false, keyOPressed = false;
bool keyPPressed = false;
bool isRightMousePressed = false;
float lastMouseY = 0.0f;
bool isCursorFree = false;
bool keyPTogglePressed = false;
bool keyCtrlOPressed = false;

int main() {
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Inicializar en pantalla completa 
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Scene In-Door", primaryMonitor, NULL);*/
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Scene In-Door", NULL, NULL);
    if (!window) {
        std::cerr << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Scene scene;
    glfwSetWindowUserPointer(window, &scene);

    UIManager uiManager;
    uiManager.Init(window);
      
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool wasSelected = false;

    while (!glfwWindowShouldClose(window)) {
        bool shouldShowCursor = (scene.selectedModel != nullptr) || isCursorFree;
        
        if (shouldShowCursor && !wasSelected) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true; 
        } else if (!shouldShowCursor && wasSelected) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        wasSelected = shouldShowCursor;

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);
        uiManager.NewFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        scene.Render(camera);
        uiManager.Render(&scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    uiManager.Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(0, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(1, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(2, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(3, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(4, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(5, deltaTime); 
  
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));  
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyPTogglePressed) {
        if (scene->selectedModel == nullptr) {
            isCursorFree = !isCursorFree;
        }
        keyPTogglePressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        keyPTogglePressed = false;
    }

    bool isCtrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
                         glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
                         
    if (isCtrlPressed && glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !keyCtrlOPressed) {
        const char* filterPatterns[1] = { "*.obj" };
        const char* filepath = tinyfd_openFileDialog("Importar Modelo 3D", "", 1, filterPatterns, "Modelos 3D", 0);
        
        if (filepath) {
            int maxID = 10;
            for (const auto& m : scene->models) {
                if (m.pickingID > maxID) maxID = m.pickingID;
            }
            
            scene->models.emplace_back(filepath);
            scene->models.back().SetPickingID(maxID + 1);
        }
        keyCtrlOPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        keyCtrlOPressed = false;
    }
    
    if (scene->selectedModel && scene->selectedModel->isLight) {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) scene->selectedModel->lightingType = 0; 
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) scene->selectedModel->lightingType = 1; 
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) scene->selectedModel->lightingType = 2; 

        keyRPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        keyGPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
        keyBPressed = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;

        keyJPressed = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
        keyKPressed = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
        keyLPressed = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

        keyUPressed = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
        keyIPressed = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
        keyOPressed = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyPPressed) { 
            scene->selectedModel->w = (scene->selectedModel->w == 1.0f) ? 0.0f : 1.0f;
            keyPPressed = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            keyPPressed = false;
        }   
    }
  
    static bool prevClickState = false;
    bool isLeftClicked = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    
    if (isLeftClicked && !prevClickState && !ImGui::GetIO().WantCaptureMouse) {
        scene->RenderPickingPass(camera);
        glBindFramebuffer(GL_FRAMEBUFFER, scene->pickingFBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        int windowWidth = 1024; 
        int windowHeight = 768;
        int xCenter = windowWidth / 2;
        int yCenter = windowHeight / 2;
        int yInverted = windowHeight - yCenter;

        unsigned char pixel[3];
        glReadPixels(xCenter, yInverted, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel); 
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        int selectedID = pixel[0] + (pixel[1] * 256) + (pixel[2] * 256 * 256);
        for (auto& model : scene->models) model.isSelected = false;
        for (int i = 0; i < scene->activeLights; i++) scene->lightSpheres[i].isSelected = false;
        scene->selectedModel = nullptr;

        if (selectedID != 0) { 
            bool found = false;
            for (auto& model : scene->models) {
                if (model.pickingID == selectedID) {
                    model.isSelected = true;
                    scene->selectedModel = &model;
                    found = true;
                    break;
                }
            }
            if (!found) {
                for (int i = 0; i < scene->activeLights; i++) {
                    if (scene->lightSpheres[i].pickingID == selectedID) {
                        scene->lightSpheres[i].isSelected = true;
                        scene->selectedModel = &scene->lightSpheres[i];
                        break;
                    }
                }
            }
        }
        
        prevClickState = true;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        prevClickState = false;
    }

    keyXPressed = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
    keyYPressed = glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS;
    keyZPressed = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;

    isMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    isRightMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    static bool rightPressed = false;
    static bool leftPressed = false;

    auto handleKeyPress = [&](int key, bool& pressed, int change) {
        if (glfwGetKey(window, key) == GLFW_PRESS && !pressed) {
            int prevLights = scene->activeLights;
            scene->activeLights = std::clamp(scene->activeLights + change, 1, 10);

            if (change > 0 && scene->activeLights > prevLights) {
                Model& newLight = scene->lightSpheres[prevLights];
                Model baseLight = scene->lightSpheres[0]; 
                newLight = baseLight; 
                
                newLight.position = scene->lightSpheres[prevLights - 1].position + glm::vec3(0.25f, 0.0f, 0.0f);
                newLight.CalculateAABB();
            }
            pressed = true;
        }
        else if (glfwGetKey(window, key) == GLFW_RELEASE) {
            pressed = false;
        }
    };

    handleKeyPress(GLFW_KEY_RIGHT, rightPressed, 1); 
    handleKeyPress(GLFW_KEY_LEFT, leftPressed, -1); 
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
        if (ImGui::GetIO().WantCaptureMouse) {
            lastX = xpos;
            lastY = ypos;
            return;
        }
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));

    if (isRightMousePressed && scene->selectedModel && !scene->selectedModel->isLight) {
        if (scene->selectedModel) {
            float sensitivity = 0.01f; 
            float scaleFactor = 1.0f + yoffset * sensitivity;

            if (!keyXPressed && !keyYPressed && !keyZPressed)
                scene->selectedModel->scale *= scaleFactor;

            if (keyXPressed) scene->selectedModel->scale.x *= scaleFactor;
            else if (keyYPressed) scene->selectedModel->scale.y *= scaleFactor;
            else if (keyZPressed) scene->selectedModel->scale.z *= scaleFactor;
                     
            scene->selectedModel->CalculateAABB(); 
        }
    } else if (isMousePressed) {
        if (scene->selectedModel) {
            float sensitivity = 0.01f; 
            float speed = 0.05f; 

            glm::vec3 delta(
                keyXPressed ? xoffset * sensitivity : 0.0f,
                keyYPressed ? yoffset * sensitivity : 0.0f,
                keyZPressed ? yoffset * sensitivity : 0.0f
            );
            scene->selectedModel->position += delta;

            if (!keyXPressed && !keyYPressed && !keyZPressed && !scene->selectedModel->isLight) {
                glm::quat rotY = glm::angleAxis(glm::radians(xoffset * speed * 50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::quat rotX = glm::angleAxis(glm::radians(yoffset * speed * 50.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
                scene->selectedModel->rotation = rotY * scene->selectedModel->rotation * rotX;             
            }

            if (scene->selectedModel && scene->selectedModel->isLight) {
                scene->lightPosition = scene->selectedModel->position;
            }
            scene->selectedModel->CalculateAABB(); 

            if (keyRPressed || keyGPressed || keyBPressed ||
                keyJPressed || keyKPressed || keyLPressed ||
                keyUPressed || keyIPressed || keyOPressed) {

                float delta = yoffset * sensitivity;

                if (keyRPressed) scene->selectedModel->lightDiffuse.r = glm::clamp(scene->selectedModel->lightDiffuse.r + delta, 0.0f, 1.0f);
                if (keyGPressed) scene->selectedModel->lightDiffuse.g = glm::clamp(scene->selectedModel->lightDiffuse.g + delta, 0.0f, 1.0f);
                if (keyBPressed) scene->selectedModel->lightDiffuse.b = glm::clamp(scene->selectedModel->lightDiffuse.b + delta, 0.0f, 1.0f);

                if (keyJPressed) scene->selectedModel->lightSpecular.r = glm::clamp(scene->selectedModel->lightSpecular.r + delta, 0.0f, 1.0f);
                if (keyKPressed) scene->selectedModel->lightSpecular.g = glm::clamp(scene->selectedModel->lightSpecular.g + delta, 0.0f, 1.0f);
                if (keyLPressed) scene->selectedModel->lightSpecular.b = glm::clamp(scene->selectedModel->lightSpecular.b + delta, 0.0f, 1.0f);

                if (keyUPressed) scene->selectedModel->attenuation.x = glm::max(scene->selectedModel->attenuation.x + delta, 0.0f);
                if (keyIPressed) scene->selectedModel->attenuation.y = glm::max(scene->selectedModel->attenuation.y + delta, 0.0f);
                if (keyOPressed) scene->selectedModel->attenuation.z = glm::max(scene->selectedModel->attenuation.z + delta, 0.0f);
            }
        }     
    }
    else if (scene->selectedModel == nullptr && !isCursorFree) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    } 
}