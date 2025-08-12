#pragma once
#include <GL/glew.h>
#include <string>

struct MeshData {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLsizei indexCount;
};

MeshData loadOBJ(const std::string& path);
