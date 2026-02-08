#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

// Struktura za 3D model
struct Model {
    unsigned int VAO;
    unsigned int VBO;
    int vertexCount;
    
    Model() : VAO(0), VBO(0), vertexCount(0) {}
};

int endProgram(std::string message);
unsigned int createShader(const char* vsSource, const char* fsSource);
unsigned loadImageToTexture(const char* filePath);
GLFWcursor* loadImageToCursor(const char* filePath);
Model loadOBJModel(const char* filePath);