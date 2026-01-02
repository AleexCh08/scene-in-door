#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include "Core/Camera.h"
#include "Scene/Scene.h"

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

int main() {
    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    // Configurar GLFW
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crear ventana
    GLFWwindow* window = glfwCreateWindow(800, 600, "Scene In-Door", NULL, NULL);
    if (!window) {
        std::cerr << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD" << std::endl;
        return -1;
    }

    // Configurar callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Habilitar el cursor oculto y capturado
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Configurar OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Crear escena
    Scene scene;
    glfwSetWindowUserPointer(window, &scene);
      
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Bucle de renderizado
    while (!glfwWindowShouldClose(window)) {
        // Calcular deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Procesar entrada
        processInput(window, deltaTime);

        // Limpiar buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Renderizar escena
        scene.Render(camera);

        // Intercambiar buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpiar y cerrar
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// Callback para redimensionar la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Procesar entrada del teclado
void processInput(GLFWwindow* window, float deltaTime) {
    // Cerrar y salir
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movimiento de la cámara con WASDQE
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(0, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(1, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(2, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(3, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(4, deltaTime); 
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(5, deltaTime); 
  
    Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));   
    if (scene->selectedModel && scene->selectedModel->isLight) {
        // Eleccion de la iluminacion
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // 1: Iluminacion lambert
            scene->selectedModel->lightingType = 0; 
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // 2: Iluminacion Phong
            scene->selectedModel->lightingType = 1; 
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) // 3: Iluminacion Blinn-phong
            scene->selectedModel->lightingType = 2; 

        // Modificar Ld (R, G, B)
        keyRPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        keyGPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
        keyBPressed = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;

        // Modificar Ls (J, K, L)
        keyJPressed = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
        keyKPressed = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
        keyLPressed = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

        // Modificar atenuacion (U, I, O)
        keyUPressed = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
        keyIPressed = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
        keyOPressed = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;

        // Cambiar tipo de luz (P)
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyPPressed) { // Luz puntual o direccional
            scene->selectedModel->w = (scene->selectedModel->w == 1.0f) ? 0.0f : 1.0f;
            keyPPressed = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            keyPPressed = false;
        }   
    }
  
    // Detectar clic izquierdo
    static bool prevClickState = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !prevClickState) {
        glm::vec3 rayStart = camera.Position;
        glm::vec3 rayDir = glm::normalize(camera.Front); 

        float closestT = FLT_MAX;
        Model* closestModel = nullptr;

        for (auto& model : scene->models) {
            if (model.isRoom) continue;
            float tNear, tFar;
            if (scene->RayAABBIntersection(rayStart, rayDir, model, tNear, tFar)) {
                if (tNear < closestT && tNear >= 0.0f) {
                    closestT = tNear;
                    closestModel = &model;
                }
            }
        }

        for (int i = 0; i < scene->activeLights; i++) { 
            Model& light = scene->lightSpheres[i];
            float tNear, tFar;
            if (scene->RayAABBIntersection(rayStart, rayDir, light, tNear, tFar)) {
                if (tNear < closestT && tNear >= 0.0f) {
                    closestT = tNear;
                    closestModel = &light;
                }
            }
        }

        for (auto& model : scene->models) {
            model.isSelected = false;
        }
        for (int i = 0; i < scene->activeLights; i++) {
            auto& light = scene->lightSpheres[i];
            light.isSelected = false;
        }
        if (closestModel) {
            closestModel->isSelected = true;
            scene->selectedModel = closestModel;
        }
        scene->selectedModel = closestModel;
        prevClickState = true;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        prevClickState = false;
    }

    // Detectar estado de las teclas X/Y/Z
    keyXPressed = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
    keyYPressed = glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS;
    keyZPressed = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;

    // Detectar estado del botón del mouse
    isMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    isRightMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Añadir y eliminar luces
    static bool rightPressed = false;
    static bool leftPressed = false;

    auto handleKeyPress = [&](int key, bool& pressed, int change) {
        if (glfwGetKey(window, key) == GLFW_PRESS && !pressed) {
            int prevLights = scene->activeLights;
            scene->activeLights = std::clamp(scene->activeLights + change, 1, 10);

            if (change > 0 && scene->activeLights > prevLights) {
                Model& newLight = scene->lightSpheres[prevLights];
                newLight = Model();
                newLight.isLight = true;
                newLight.position = scene->lightSpheres[prevLights - 1].position + glm::vec3(0.25f, 0.0f, 0.0f);

                scene->generateSphere(0.1f, 20, 20, newLight.vertices, newLight.indices);
                std::vector<float> vertexData;
                for (size_t j = 0; j < newLight.vertices.size() / 8; ++j) {
                    for (int k = 0; k < 8; ++k) {
                        vertexData.push_back(newLight.vertices[j * 8 + k]);
                    }
                }
                scene->SetupModelBuffers(newLight, vertexData);
                scene->CalculateAABB(newLight, true);
            }
            pressed = true;
        }
        else if (glfwGetKey(window, key) == GLFW_RELEASE) {
            pressed = false;
        }
    };

    handleKeyPress(GLFW_KEY_RIGHT, rightPressed, 1); // RIGHT: añadir una luz
    handleKeyPress(GLFW_KEY_LEFT, leftPressed, -1); // LEFT: quitar una luz

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) { // M: Salvar la escena actual
        scene->SaveScene("scene.json");
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) { // N: Cargar la escena
        scene->LoadScene("scene.json");
    }
}

// Callback para el movimiento del ratón
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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
            float sensitivity = 0.01f; // Velocidad de escalamiento
            float scaleFactor = 1.0f + yoffset * sensitivity;

            // Aplicar escala en todas las dimensiones
            if (!keyXPressed && !keyYPressed && !keyZPressed)
                scene->selectedModel->scale *= scaleFactor;

            // Aplicar escala en la dimensión correspondiente
            if (keyXPressed) {
                scene->selectedModel->scale.x *= scaleFactor;
            }
            else if (keyYPressed) {
                scene->selectedModel->scale.y *= scaleFactor;
            }
            else if (keyZPressed) {
                scene->selectedModel->scale.z *= scaleFactor;
            }         
            scene->CalculateAABB(*scene->selectedModel, false);
        }
    } else if (isMousePressed) {
        if (scene->selectedModel) {
            float sensitivity = 0.01f; // Velocidad de desplazamiento
            float speed = 0.05f; // Velocidad de rotacion

            glm::vec3 delta(
                keyXPressed ? xoffset * sensitivity : 0.0f,
                keyYPressed ? yoffset * sensitivity : 0.0f,
                keyZPressed ? yoffset * sensitivity : 0.0f
            );
            scene->selectedModel->position += delta;

            if (!keyXPressed && !keyYPressed && !keyZPressed && !scene->selectedModel->isLight) {
                scene->selectedModel->rotation.y += xoffset * speed; // Rotación en Y (horizontal)
                scene->selectedModel->rotation.x += yoffset * speed; // Rotación en X (vertical)              
            }

            if (scene->selectedModel && scene->selectedModel->isLight) {
                scene->lightPosition = scene->selectedModel->position;
            }
            scene->CalculateAABB(*scene->selectedModel, scene->selectedModel->isLight);

            // Modificar parámetros con el eje Y del mouse
            if (keyRPressed || keyGPressed || keyBPressed ||
                keyJPressed || keyKPressed || keyLPressed ||
                keyUPressed || keyIPressed || keyOPressed) {

                float delta = yoffset * sensitivity;

                // Ld (R, G, B)
                if (keyRPressed) scene->selectedModel->lightDiffuse.r = glm::clamp(scene->selectedModel->lightDiffuse.r + delta, 0.0f, 1.0f);
                if (keyGPressed) scene->selectedModel->lightDiffuse.g = glm::clamp(scene->selectedModel->lightDiffuse.g + delta, 0.0f, 1.0f);
                if (keyBPressed) scene->selectedModel->lightDiffuse.b = glm::clamp(scene->selectedModel->lightDiffuse.b + delta, 0.0f, 1.0f);

                // Ls (J, K, L)
                if (keyJPressed) scene->selectedModel->lightSpecular.r = glm::clamp(scene->selectedModel->lightSpecular.r + delta, 0.0f, 1.0f);
                if (keyKPressed) scene->selectedModel->lightSpecular.g = glm::clamp(scene->selectedModel->lightSpecular.g + delta, 0.0f, 1.0f);
                if (keyLPressed) scene->selectedModel->lightSpecular.b = glm::clamp(scene->selectedModel->lightSpecular.b + delta, 0.0f, 1.0f);

                // Atenuación (U, I, O)
                if (keyUPressed) scene->selectedModel->attenuation.x = glm::max(scene->selectedModel->attenuation.x + delta, 0.0f);
                if (keyIPressed) scene->selectedModel->attenuation.y = glm::max(scene->selectedModel->attenuation.y + delta, 0.0f);
                if (keyOPressed) scene->selectedModel->attenuation.z = glm::max(scene->selectedModel->attenuation.z + delta, 0.0f);
            }
        }     
    }
    else {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }  
}