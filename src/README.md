# Motor Gráfico 3D en OpenGL

Un motor gráfico 3D interactivo y renderizador en tiempo real desarrollado desde cero en C++ utilizando OpenGL moderno (Core Profile 3.3). Diseñado con una arquitectura modular, permite la importación dinámica de modelos 3D, manipulación de transformaciones en tiempo real, iluminación dinámica y gestión de escenas con guardado/carga mediante serialización JSON.

## 🚀 Características Principales

* **Importación Dinámica de Modelos:** Carga de modelos `.obj` en tiempo real mediante diálogos de archivo nativos del sistema operativo.
* **Sistema de Selección (Color Picking):** Selección de objetos en escena con precisión a nivel de píxel mediante un Framebuffer Object (FBO) invisible, con generación visual de Bounding Boxes.
* **Iluminación Dinámica Avanzada:** Soporte para hasta 10 luces puntuales activas modificables en tiempo real. Incluye selección de modelo de sombreado (Lambert, Phong, Blinn-Phong), color difuso/especular y atenuación lineal/cuadrática.
* **Interfaz Gráfica de Usuario (ImGUI):** Herramientas de inspección integradas (Inspector de Propiedades y Menú Global) para manipular posición, rotación, escala (con límites de seguridad) y variables de iluminación.
* **Gestión de Escenas:** Guardado y carga del estado completo de la escena (modelos, luces y posición de la cámara) a archivos `.json` mediante serialización.
* **Navegación:** Cámara en primera persona (First-Person Camera) estilo *fly-cam* con alternancia fluida entre control de vista y control de interfaz (cursor libre).
* **Entorno:** Renderizado de Skybox inmersivo.

## 🛠️ Tecnologías y Librerías Utilizadas

El proyecto fue construido utilizando herramientas estándar de la industria para el desarrollo gráfico en C++:

* **C++17:** Lenguaje de programación principal.
* **OpenGL 3.3:** API gráfica de bajo nivel.
* **GLFW:** Creación de ventanas, contextos y manejo de entradas (teclado/ratón).
* **GLAD:** Cargador de punteros y extensiones de OpenGL.
* **ImGui:** Interfaz gráfica de usuario en modo inmediato (Immediate Mode GUI).
* **Assimp:** Biblioteca para la importación estandarizada de modelos 3D multiformato.
* **GLM (OpenGL Mathematics):** Biblioteca matemática para álgebra lineal y transformaciones 3D.
* **stb_image:** Carga de texturas e imágenes.
* **nlohmann/json:** Serialización de datos para el guardado/carga de escenas.
* **tinyfiledialogs:** Llamadas a la API nativa del sistema operativo (Win32/GTK) para cuadros de diálogo de archivos portables.
* **CMake:** Sistema de construcción y empaquetado del proyecto.

## 🎮 Controles de la Aplicación

| Tecla / Acción | Descripción |
| :--- | :--- |
| **W, A, S, D** | Movimiento de la cámara (Adelante, Izquierda, Atrás, Derecha). |
| **Q / E** | Movimiento vertical de la cámara (Abajo / Arriba). |
| **P** | Alternar entre el control de la cámara (mira) y el cursor libre para la UI. |
| **Ctrl + O** | Atajo rápido para abrir el cuadro de diálogo e importar un modelo 3D. |
| **Click Izquierdo** | Seleccionar un modelo o luz en la escena. |
| **Esc** | Cerrar la aplicación de forma segura. |

## ⚙️ Compilación y Ejecución

Este proyecto utiliza **CMake** para la configuración del entorno de construcción. 

1. Clona el repositorio.
2. Crea una carpeta de construcción (`mkdir build && cd build`).
3. Ejecuta CMake (`cmake ..`).
4. Compila el proyecto (`cmake --build .`).

*Nota: Asegúrate de tener instaladas las dependencias y librerías dinámicas necesarias en tu sistema (como Assimp y GLFW) según tu entorno de desarrollo.*

## 👨‍💻 Desarrollador

Desarrollado por **AleexCh**.