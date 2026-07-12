cmake_minimum_required(VERSION 3.10)

project(MotorGrafico VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(${INCLUDE_DIR})
include_directories(${SRC_DIR}/imgui)

file(GLOB_RECURSE SOURCES 
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_options(${PROJECT_NAME} PRIVATE "-mwindows")
find_library(GLFW_LIB NAMES glfw3 PATHS ${LIB_DIR} NO_DEFAULT_PATH)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        ${GLFW_LIB}
        opengl32
        gdi32
        user32
        kernel32
        shell32
        assimp
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIB} GL dl assimp)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/objects
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/objects

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Textures
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/src/Textures
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/objects.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/objects.json

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/scene.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/scene.json

    COMMENT "Copiando carpeta objects, texturas y archivos JSON al directorio del ejecutable..."
)