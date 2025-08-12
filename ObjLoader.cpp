#include "ObjLoader.h"
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
#include <algorithm>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;

    // stable ordering for std::map key
    bool operator<(const Vertex& o) const {
        return std::tie(position.x, position.y, position.z,
                        texCoord.x,  texCoord.y,
                        normal.x,    normal.y,    normal.z)
             < std::tie(o.position.x, o.position.y, o.position.z,
                        o.texCoord.x,  o.texCoord.y,
                        o.normal.x,    o.normal.y,    o.normal.z);
    }
};

static inline int toIndex(const std::string& token, int count) {
    if (token.empty()) return -1;
    int v = std::stoi(token);
    // OBJ supports negative indices (relative to end)
    return (v > 0) ? (v - 1) : (count + v);
}

MeshData loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return {0,0,0,0};
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    std::vector<Vertex>              vertices;       // unique vertex buffer
    std::map<Vertex, GLuint>         vertexToIndex;  // vertex -> index
    std::vector<GLuint>              indices;        // triangle indices

    std::string line, tag;
    while (std::getline(file, line)) {
        std::istringstream s(line);
        if (!(s >> tag)) continue;

        if (tag == "v") {
            glm::vec3 p; s >> p.x >> p.y >> p.z; positions.push_back(p);
        } else if (tag == "vt") {
            glm::vec2 t; s >> t.x >> t.y;       texCoords.push_back(t);
        } else if (tag == "vn") {
            glm::vec3 n; s >> n.x >> n.y >> n.z; normals.push_back(n);
        } else if (tag == "f") {
            // collect all per-vertex tokens for this face (tri, quad, n-gon)
            std::vector<std::string> toks;
            std::string tok;
            while (s >> tok) toks.push_back(tok);
            if (toks.size() < 3) continue;

            auto parseVert = [&](const std::string& t) -> Vertex {
                // formats: a/b/c, a//c, a/b, a   (1-based; negatives allowed)
                std::istringstream fs(t);
                std::string A,B,C;
                std::getline(fs, A, '/');
                std::getline(fs, B, '/');
                std::getline(fs, C, '/');

                int pi = toIndex(A, (int)positions.size());
                int ti = toIndex(B, (int)texCoords.size());
                int ni = toIndex(C, (int)normals.size());

                // clamp to valid ranges; provide safe defaults
                if (pi < 0 || pi >= (int)positions.size()) pi = 0;
                glm::vec2 uv(0.0f);
                glm::vec3 nrm(0.0f, 0.0f, 1.0f);
                if (ti >= 0 && ti < (int)texCoords.size()) uv = texCoords[ti];
                if (ni >= 0 && ni < (int)normals.size())   nrm = normals[ni];

                return { positions[pi], uv, nrm };
            };

            auto add = [&](const Vertex& vtx){
                auto it = vertexToIndex.find(vtx);
                if (it == vertexToIndex.end()) {
                    GLuint idx = (GLuint)vertices.size();
                    vertexToIndex[vtx] = idx;
                    vertices.push_back(vtx);
                    indices.push_back(idx);
                } else {
                    indices.push_back(it->second);
                }
            };

            // triangle fan: (0, i-1, i)  for i = 2..n-1
            Vertex v0 = parseVert(toks[0]);
            Vertex v1 = parseVert(toks[1]);
            for (size_t i = 2; i < toks.size(); ++i) {
                Vertex v2 = parseVert(toks[i]);
                add(v0); add(v1); add(v2);
                v1 = v2;
            }
        }
    }

    // Interleave: pos(3) | uv(2) | normal(3)
    std::vector<float> interleaved;
    interleaved.reserve(vertices.size() * 8);
    for (const auto& v : vertices) {
        interleaved.push_back(v.position.x);
        interleaved.push_back(v.position.y);
        interleaved.push_back(v.position.z);
        interleaved.push_back(v.texCoord.x);
        interleaved.push_back(v.texCoord.y);
        interleaved.push_back(v.normal.x);
        interleaved.push_back(v.normal.y);
        interleaved.push_back(v.normal.z);
    }

    GLuint VAO=0, VBO=0, EBO=0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(interleaved.size()*sizeof(float)), interleaved.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size()*sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(2); // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));

    glBindVertexArray(0);

    return { VAO, VBO, EBO, (GLsizei)indices.size() };
}
