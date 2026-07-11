#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>

class Shader {
public:
    Shader(bool useDefault = true);
    Shader(const char* vertexSource, const char* fragmentSource);
    void Use() const;
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    GLuint GetID() const { return programID; }

private:
    GLuint programID;
    static GLuint compileShader(const char* source, GLenum type);
    static GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
};

// Shader del modelo
inline const char* vertexShaderSource =
R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec2 aTexCoords;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoords;

    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoords = aTexCoords;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

inline const char* fragmentShaderSource =
R"(
    #version 330 core
    out vec4 FragColor;

    struct Light {
        vec3 position;
        vec3 ambient;
        vec3 diffuse; 
        vec3 specular;    
        vec3 attenuation; 
        float w;          
        int lightingType; 
    };

    struct Material {
        sampler2D texture_diffuse1;
        sampler2D texture_specular1;
        float shininess;
    };

    uniform Light lights[10]; 
    uniform int numLights;
    uniform Material material;
    uniform vec3 viewPos;

    uniform bool isLightSource;
    uniform vec3 lightColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

    void main() {
        if (isLightSource) {
            FragColor = vec4(lightColor, 1.0);
        } else {
            vec3 norm = normalize(Normal);
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 result = vec3(0.0);

            // Muestrear las texturas 
            vec3 diffuseTexColor = vec3(texture(material.texture_diffuse1, TexCoords));
            vec3 specularTexColor = vec3(texture(material.texture_specular1, TexCoords));

            for(int i = 0; i < numLights; ++i) {
                vec3 lightDir;
                if (lights[i].w == 1.0) { 
                    lightDir = normalize(lights[i].position - FragPos);
                } else { 
                    lightDir = normalize(-lights[i].position); 
                }

                float attenuation = 1.0;
                if (lights[i].w == 1.0) {
                    float distance = length(lights[i].position - FragPos);
                    attenuation = 1.0 / (lights[i].attenuation.x + 
                                        lights[i].attenuation.y * distance + 
                                        lights[i].attenuation.z * (distance * distance));
                }

                vec3 ambient = lights[i].ambient * diffuseTexColor;

                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = lights[i].diffuse * diff * diffuseTexColor;

                vec3 specular = vec3(0.0);
                if (lights[i].lightingType != 0 && diff > 0.0) {
                    if (lights[i].lightingType == 1) { // Phong
                        vec3 reflectDir = reflect(-lightDir, norm);
                        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
                        specular = lights[i].specular * spec * specularTexColor;
                    } else if (lights[i].lightingType == 2) { // Blinn-Phong
                        vec3 halfwayDir = normalize(lightDir + viewDir);
                        float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
                        specular = lights[i].specular * spec * specularTexColor;
                    }
                }

                ambient *= attenuation;
                diffuse *= attenuation;
                specular *= attenuation;

                result += ambient + diffuse + specular;
            }
            FragColor = vec4(result, 1.0);
        }
    }
)";

// Shader de la mira 
inline const char* crosshairVertexShader =
R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

inline const char* crosshairFragmentShader =
R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)";

// Shader para Bounding Box
inline const char* bboxVertexShader =
R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

inline const char* bboxFragmentShader =
R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)";

inline Shader::Shader(const char* vertexSource, const char* fragmentSource) {
    programID = createShaderProgram(vertexSource, fragmentSource);
}

inline Shader::Shader(bool useDefault) {
    if (useDefault) {
        programID = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    }
    else {
        programID = 0;
    }
}

inline void Shader::Use() const {
    glUseProgram(programID);
}

inline void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}

inline void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

inline void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

inline void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

inline void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

inline GLuint Shader::compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error compilando shader: " << infoLog << std::endl;
    }

    return shader;
}

inline GLuint Shader::createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0) {
        std::cerr << "Error al compilar shaders" << std::endl;
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error enlazando programa de shaders: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
