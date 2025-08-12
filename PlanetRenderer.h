#ifndef PLANET_RENDERER_H
#define PLANET_RENDERER_H

#include <vector>
#include <string>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

// Simple mesh container for generated planets/rings
struct Planet {
    unsigned int VAO = 0, VBO = 0, EBO = 0, textureID = 0;
    std::vector<float> vertices;         // interleaved: pos(3), uv(2), normal(3)  => 8 floats per vertex
    std::vector<unsigned int> indices;
};

// -------------------------------
// Sphere (planet) mesh generator
// -------------------------------
// Builds a unit sphere centered at origin, oriented with Z up.
// Produces interleaved vertices: [x,y,z, u,v, nx,ny,nz]
static void generateSphereMesh(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    int sectorCount = 64,     // longitude (around Z)
    int stackCount  = 32      // latitude
) {
    vertices.clear();
    indices.clear();

    const float PI = 3.14159265359f;
    const float sectorStep = 2.f * PI / sectorCount;
    const float stackStep  = PI / stackCount;

    // Vertices with normals = position (unit sphere)
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2.f - i * stackStep; // from +pi/2 to -pi/2
        float xy = cosf(stackAngle);
        float z  = sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            float s = (float)j / sectorCount;
            float t = (float)i / stackCount;

            // position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // uv
            vertices.push_back(s);
            vertices.push_back(t);
            // normal (same as position for unit sphere)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Indices
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

// ----------------------------------------
// Ring (plane) mesh for Saturn-like rings
// ----------------------------------------
// Simple quad grid in XY plane (Z = 0), normal = +Z.
// Interleaved vertices: [x,y,0, u,v, 0,0,1]
static void generatePlaneMesh(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    int widthSegments  = 32,
    int heightSegments = 32
) {
    vertices.clear();
    indices.clear();

    // Build [0,1] UV-mapped plane spanning [-1,1] in X and Y
    for (int i = 0; i <= heightSegments; ++i) {
        float v  = (float)i / heightSegments;
        float y  = -1.f + 2.f * v;

        for (int j = 0; j <= widthSegments; ++j) {
            float u = (float)j / widthSegments;
            float x = -1.f + 2.f * u;

            // position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.f);
            // uv
            vertices.push_back(u);
            vertices.push_back(v);
            // normal +Z
            vertices.push_back(0.f);
            vertices.push_back(0.f);
            vertices.push_back(1.f);
        }
    }

    for (int i = 0; i < heightSegments; ++i) {
        for (int j = 0; j < widthSegments; ++j) {
            int topLeft     = i * (widthSegments + 1) + j;
            int topRight    = topLeft + 1;
            int bottomLeft  = (i + 1) * (widthSegments + 1) + j;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

// -------------------------------
// Texture loader helper (declared elsewhere in your project)
// -------------------------------
extern unsigned int loadTexture(const char* path);

// -------------------------------------------
// Initialization for planets (sphere geometry)
// -------------------------------------------
static void initPlanet(Planet& planet, const std::string& texturePath) {
    generateSphereMesh(planet.vertices, planet.indices);

    glGenVertexArrays(1, &planet.VAO);
    glGenBuffers(1, &planet.VBO);
    glGenBuffers(1, &planet.EBO);

    glBindVertexArray(planet.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, planet.VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 planet.vertices.size() * sizeof(float),
                 planet.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 planet.indices.size() * sizeof(unsigned int),
                 planet.indices.data(),
                 GL_STATIC_DRAW);

    // pos(0), uv(1), normal(2) with stride 8 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Texture
    planet.textureID = loadTexture(texturePath.c_str());

    glBindVertexArray(0);
}

// -------------------------------------------
// Initialization for rings (plane geometry)
// -------------------------------------------
static void initRings(Planet& rings, const std::string& texturePath) {
    generatePlaneMesh(rings.vertices, rings.indices);

    glGenVertexArrays(1, &rings.VAO);
    glGenBuffers(1, &rings.VBO);
    glGenBuffers(1, &rings.EBO);

    glBindVertexArray(rings.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, rings.VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 rings.vertices.size() * sizeof(float),
                 rings.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rings.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 rings.indices.size() * sizeof(unsigned int),
                 rings.indices.data(),
                 GL_STATIC_DRAW);

    // pos(0), uv(1), normal(2) with stride 8 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Texture (rings texture may be PNG with alpha; your loader handles formats)
    rings.textureID = loadTexture(texturePath.c_str());

    glBindVertexArray(0);
}

// -------------------------------------------
// Render helpers
// -------------------------------------------
static void renderPlanet(Planet& planet, Shader& shader,
                         glm::vec3 position,
                         float scale = 1.0f,
                         float spin = 0.0f,
                         float tilt = 0.0f) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // fix orientation
    model = glm::rotate(model, glm::radians(tilt),    glm::vec3(0.0f, 0.0f, 1.0f)); // axial tilt
    model = glm::rotate(model, glm::radians(spin),    glm::vec3(0.0f, 0.0f, 1.0f)); // spin
    model = glm::scale(model, glm::vec3(scale));

    shader.setMat4("model", model);

    glBindTexture(GL_TEXTURE_2D, planet.textureID);
    glBindVertexArray(planet.VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)planet.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

static void renderRings(Planet& rings, Shader& shader,
                        glm::vec3 position,
                        float scale = 1.0f,
                        float spin = 0.0f,
                        float tilt = 0.0f) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    // rings lie in XY; tilt/spin around Z
    model = glm::rotate(model, glm::radians(tilt), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(spin), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale));

    shader.setMat4("model", model);

    glBindTexture(GL_TEXTURE_2D, rings.textureID);
    glBindVertexArray(rings.VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)rings.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#endif
