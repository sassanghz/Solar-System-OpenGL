#ifndef PLANET_RENDERER_H
#define PLANET_RENDERER_H

#include <vector>
#include <string>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"


struct Planet {
    unsigned int VAO = 0, VBO, EBO, textureID;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

void generateSphereMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int sectorCount = 36, int stackCount = 18) {
    const float PI = 3.1415926f;
    float x, y, z, xy;
    float s, t;

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;
        xy = cosf(stackAngle);
        z = sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

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


void generatePlaneMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int widthSegments = 10, int heightSegments = 10) {
    vertices.clear();
    indices.clear();
    
    // Generate vertices for a plane from -1 to 1 in X and Z, Y = 0
    for (int i = 0; i <= heightSegments; ++i) {
        for (int j = 0; j <= widthSegments; ++j) {
            float x = -1.0f + (2.0f * j / widthSegments);  // X from -1 to 1
            float y = 0.0f;                                 // Flat plane at Y = 0
            float z = -1.0f + (2.0f * i / heightSegments); // Z from -1 to 1
            
            float s = (float)j / widthSegments;             // Texture U coordinate
            float t = (float)i / heightSegments;            // Texture V coordinate
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }
    
    // Generate indices for triangles
    for (int i = 0; i < heightSegments; ++i) {
        for (int j = 0; j < widthSegments; ++j) {
            int topLeft = i * (widthSegments + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * (widthSegments + 1) + j;
            int bottomRight = bottomLeft + 1;
            
            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

void initPlanet(Planet& planet, const std::string& texturePath) {
    generateSphereMesh(planet.vertices, planet.indices);

    glGenVertexArrays(1, &planet.VAO);
    glGenBuffers(1, &planet.VBO);
    glGenBuffers(1, &planet.EBO);

    glBindVertexArray(planet.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, planet.VBO);
    glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(float), &planet.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    glGenTextures(1, &planet.textureID);
    glBindTexture(GL_TEXTURE_2D, planet.textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (data) {
        //NO ALPHA (JPG)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
}
void initRings(Planet& rings, const std::string& texturePath) {
    generatePlaneMesh(rings.vertices, rings.indices);
    glGenVertexArrays(1, &rings.VAO);
    glGenBuffers(1, &rings.VBO);
    glGenBuffers(1, &rings.EBO);

    glBindVertexArray(rings.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, rings.VBO);
    glBufferData(GL_ARRAY_BUFFER, rings.vertices.size() * sizeof(float), &rings.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rings.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rings.indices.size() * sizeof(unsigned int), &rings.indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    glGenTextures(1, &rings.textureID);
    glBindTexture(GL_TEXTURE_2D, rings.textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (data) {
        //WITH ALPHA (PNG)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
}


void renderPlanet(Planet& planet, Shader& shader, glm::vec3 position, float scale = 1.0f, float spin = 0.0f, float tilt = 0.0f) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //fix orientation
    model = glm::rotate(model, glm::radians(tilt), glm::vec3(0.0f, 0.0f, 1.0f));   // axial tilt
    model = glm::rotate(model, glm::radians(spin), glm::vec3(0.0f, 0.0f, 1.0f));   // spin around Z
    model = glm::scale(model, glm::vec3(scale));
    shader.setMat4("model", model);

    glBindTexture(GL_TEXTURE_2D, planet.textureID);
    glBindVertexArray(planet.VAO);
    glDrawElements(GL_TRIANGLES, planet.indices.size(), GL_UNSIGNED_INT, 0);
}

void renderRings(Planet& rings, Shader& shader, glm::vec3 position, float scale = 1.0f, float spin = 0.0f, float tilt = 0.0f) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
   // model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //fix orientation
    model = glm::rotate(model, glm::radians(tilt), glm::vec3(0.0f, 0.0f, 1.0f));   // axial tilt
    model = glm::rotate(model, glm::radians(spin), glm::vec3(0.0f, 0.0f, 1.0f));   // spin around Z
    model = glm::scale(model, glm::vec3(scale));
    shader.setMat4("model", model);

    glBindTexture(GL_TEXTURE_2D, rings.textureID);
    glBindVertexArray(rings.VAO);
    glDrawElements(GL_TRIANGLES, rings.indices.size(), GL_UNSIGNED_INT, 0);
}

#endif
